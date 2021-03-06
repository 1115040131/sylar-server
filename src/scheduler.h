//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// scheduler.h
//
// Identification: src/scheduler.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <memory>
#include <vector>

#include "fiber.h"
#include "thread.h"

namespace sylar {

class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();
    static Fiber* GetMainFiber();

    void start();
    void stop();

    template <class FiberOrCb>
    void schedule(FiberOrCb fc, int thread = -1) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }

        if (need_tickle) {
            tickle();
        }
    }

    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end) {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin, -1) || need_tickle;
                ++begin;
            }
        }

        if (need_tickle) {
            tickle();
        }
    }

protected:
    void setThis();
    void run();

    virtual void tickle();
    virtual bool stopping();
    virtual void idle();

private:
    /**
     * @brief 协程调度启动(无锁)
     */
    template <class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread) {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.emplace_back(ft);
        }
        return need_tickle;
    }

private:
    /**
     * @brief 协程/函数/线程组
     */
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr _fiber, int _thread)
            : fiber(_fiber), thread(_thread) {}

        FiberAndThread(Fiber::ptr* _fiber, int _thread)
            : thread(_thread) {
            fiber.swap(*_fiber);
        }

        FiberAndThread(std::function<void()> _cb, int _thread)
            : cb(_cb), thread(_thread) {}

        FiberAndThread(std::function<void()>* _cb, int _thread)
            : thread(_thread) {
            cb.swap(*_cb);
        }

        FiberAndThread() : thread(-1) {}

        void reset() {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    MutexType m_mutex;

    std::string m_name;
    std::vector<Thread::ptr> m_threads;  // 线程池
    std::list<FiberAndThread> m_fibers;  // 协程消息队列
    Fiber::ptr m_root_fiber;             // 主协程

protected:
    std::vector<int> m_thread_ids;                 // 协程下的线程id数组
    size_t m_thread_count = 0;                     // 线程数量
    std::atomic<size_t> m_active_thread_count{0};  // 工作线程数量
    std::atomic<size_t> m_idle_thread_count{0};    // 空闲线程数量
    bool m_stopping = true;                        // 是否正在停止
    bool m_auto_stop = false;                      // 是否自动停止
    int m_root_thread_id = 0;                      // 主线程id(use_caller)
};

}  // namespace sylar
