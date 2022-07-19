//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// scheduler.cpp
//
// Identification: src/scheduler.cpp
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#include "scheduler.h"

#include "log.h"
#include "macro.h"

namespace sylar {

static sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static thread_local Scheduler* t_scheduler = nullptr;
static thread_local Fiber* t_fiber = nullptr;

Scheduler::Scheduler(size_t threads, bool use_caller, const std::string& name)
    : m_name(name) {
    SYLAR_ASSERT(threads > 0);

    if (use_caller) {
        sylar::Fiber::GetThis();
        threads--;

        // 确保之前没有创建协程调度器
        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_root_fiber = std::make_shared<Fiber>(std::bind(&Scheduler::run, this), 0, true);
        sylar::Thread::SetName(m_name);

        t_fiber = m_root_fiber.get();
        m_root_thread_id = sylar::GetThreadId();
        m_thread_ids.emplace_back(m_root_thread_id);
    } else {
        m_root_thread_id = -1;
    }
    m_thread_count = threads;
}

Scheduler::~Scheduler() {
    SYLAR_ASSERT(m_stopping);
    if (GetThis() == this) {
        t_scheduler = nullptr;
    }
}

Scheduler* Scheduler::GetThis() {
    return t_scheduler;
}

Fiber* Scheduler::GetMainFiber() {
    return t_fiber;
}

void Scheduler::start() {
    MutexType::Lock lock(m_mutex);

    // 已启动
    if (!m_stopping) {
        return;
    }

    m_stopping = false;
    SYLAR_ASSERT(m_threads.empty());

    m_threads.reserve(m_thread_count);
    for (size_t i = 0; i < m_thread_count; ++i) {
        m_threads.emplace_back(std::make_shared<sylar::Thread>(std::bind(&Scheduler::run, this),
                                                               m_name + "_" + std::to_string(i)));
        m_thread_ids.emplace_back(m_threads[i]->getId());
    }
    lock.unlock();

    // if (m_root_fiber) {
    //     m_root_fiber->call();
    //     // m_root_fiber->swapIn();
    //     SYLAR_LOG_INFO(g_logger) << "call out " << m_root_fiber->getState();
    // }
}

void Scheduler::stop() {
    m_auto_stop = true;
    if (m_root_fiber && m_thread_count == 0 &&
        (m_root_fiber->getState() == Fiber::TERM || m_root_fiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_INFO(g_logger) << this << " stopped";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    // bool exit_on_this_fiber = false;
    // use_caller线程
    if (m_root_thread_id != -1) {
        SYLAR_ASSERT(GetThis() == this);
    } else {
        SYLAR_ASSERT(GetThis() != this);
    }

    m_stopping = true;
    for (size_t i = 0; i < m_thread_count; ++i) {
        tickle();
    }

    if (m_root_fiber) {
        tickle();

        // while (!stopping()) {
        //     if (m_root_fiber->getState() == Fiber::TERM || m_root_fiber->getState() == Fiber::EXCEPT) {
        //         m_root_fiber = std::make_shared<Fiber>(std::bind(&Scheduler::run, this), 0, true);
        //         SYLAR_LOG_INFO(g_logger) << "root fiber is term, reset";
        //         t_fiber = m_root_fiber.get();
        //     }
        //     m_root_fiber->call();
        // }
        if (!stopping()) {
            m_root_fiber->call();
        }
    }

    std::vector<Thread::ptr> thrs;
    {
        MutexType::Lock lock(m_mutex);
        thrs.swap(m_threads);
    }

    for (auto& i : thrs) {
        i->join();
    }
}

void Scheduler::run() {
    SYLAR_LOG_INFO(g_logger) << "run";
    // 将当前线程置为scheduler
    setThis();
    if (sylar::GetThreadId() != m_root_thread_id) {
        // 将主协程置为当前线程上的协程
        t_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber = std::make_shared<Fiber>(std::bind(&Scheduler::idle, this));
    Fiber::ptr cb_fiber;

    FiberAndThread ft;
    while (true) {
        ft.reset();
        bool tickle_me = false;
        bool is_active = false;
        // 从消息队列中取出应该要执行的消息
        {
            MutexType::Lock lock(m_mutex);

            auto iter = m_fibers.begin();
            while (iter != m_fibers.end()) {
                // 当前线程不是要执行的线程
                if (iter->thread != -1 && iter->thread != sylar::GetThreadId()) {
                    ++iter;
                    tickle_me = true;
                    continue;
                }

                SYLAR_ASSERT(iter->fiber || iter->cb);
                // 当前线程已经在执行
                if (iter->fiber && iter->fiber->getState() == Fiber::EXEC) {
                    ++iter;
                    continue;
                }

                ft = *iter;
                m_fibers.erase(iter);
                ++m_active_thread_count;
                is_active = true;
                break;
            }
        }

        if (tickle_me) {
            tickle();
        }

        // 如果ft为fiber类型并且状态不为TERM或EXCEPT, 则执行
        if (ft.fiber && (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT)) {
            
            ft.fiber->swapIn();
            --m_active_thread_count;

            // 切换回来后若状态为READY则继续加入消息队列
            if (ft.fiber->getState() == Fiber::READY) {
                schedule(ft.fiber);
                // 若状态为INIT, HOLD, EXEC置为HOLD
            } else if (ft.fiber->getState() != Fiber::TERM && ft.fiber->getState() != Fiber::EXCEPT) {
                ft.fiber->m_state = Fiber::HOLD;
            }
            // 结束后将ft重置
            ft.reset();

            // 如果ft为cb类型
        } else if (ft.cb) {
            if (cb_fiber) {
                cb_fiber->reset(ft.cb);
            } else {
                cb_fiber = std::make_shared<Fiber>(ft.cb);
            }
            ft.reset();

            cb_fiber->swapIn();
            --m_active_thread_count;
            if (cb_fiber->getState() == Fiber::READY) {
                schedule(cb_fiber);
                cb_fiber.reset();
            } else if (cb_fiber->getState() == Fiber::EXCEPT || cb_fiber->getState() == Fiber::TERM) {
                cb_fiber->reset(nullptr);
            } else {  // if (cb_fiber->getState() != Fiber::TERM) {
                cb_fiber->m_state = Fiber::HOLD;
                cb_fiber.reset();
            }
        } else {
            if (is_active) {
                --m_active_thread_count;
                continue;
            }
            if (idle_fiber->getState() == Fiber::TERM) {
                SYLAR_LOG_INFO(g_logger) << "idle fiber term";
                break;
            }
            ++m_idle_thread_count;
            idle_fiber->swapIn();
            --m_idle_thread_count;
            if (idle_fiber->getState() != Fiber::TERM && idle_fiber->getState() != Fiber::EXCEPT) {
                idle_fiber->m_state = Fiber::HOLD;
            }
        }
    }
}

void Scheduler::setThis() {
    t_scheduler = this;
}

void Scheduler::tickle() {
    SYLAR_LOG_INFO(g_logger) << "tickle";
}

bool Scheduler::stopping() {
    MutexType::Lock lock(m_mutex);

    return m_auto_stop && m_stopping && m_fibers.empty() && m_active_thread_count == 0;
}
void Scheduler::idle() {
    SYLAR_LOG_INFO(g_logger) << "idle";
    while (!stopping()) {
        sylar::Fiber::YieldToHold();
    }
}

}  // namespace sylar
