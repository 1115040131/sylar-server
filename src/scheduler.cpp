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

Scheduler::Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "")
    : m_name(name) {
    SYLAR_ASSERT(threads > 0);

    if (use_caller) {
        sylar::Fiber::GetThis();
        threads--;

        // 确保之前没有创建协程调度器
        SYLAR_ASSERT(GetThis() == nullptr);
        t_scheduler = this;

        m_root_fiber = std::make_shared<Fiber>(std::bind(&Scheduler::run, this));
        sylar::Thread::SetName(m_name);

        t_fiber = m_root_fiber.get();
        m_root_thread_id = sylar::GetThreadId();
        m_thread_ids.emplace_back(t_fiber);
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
}

void Scheduler::stop() {
    m_auto_stop = true;
    if (m_root_fiber && m_thread_count == 0 &&
        (m_root_fiber->getState() == Fiber::TERM || m_root_fiber->getState() == Fiber::INIT)) {
        SYLAR_LOG_NAME(g_logger) << this << " stopped";
        m_stopping = true;

        if (stopping()) {
            return;
        }
    }

    bool exit_on_this_fiber = false;
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
    }

    if (stopping()) {
        return;
    }

    // if (exit_on_this_fiber) {
    // }
}

void Scheduler::run() {
    setThis();

    if (sylar::GetThreadId() != m_root_thread_id) {
        t_fiber = Fiber::GetThis().get();
    }

    Fiber::ptr idle_fiber = std::make_shared<Fiber>;
}

void Scheduler::setThis() {
    t_scheduler = this;
}

}  // namespace sylar
