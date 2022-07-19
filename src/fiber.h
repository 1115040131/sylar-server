//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// fiber.h
//
// Identification: src/fiber.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once

#include <ucontext.h>

#include <functional>
#include <memory>

#include "thread.h"

namespace sylar {

class Scheduler;
class Fiber : public std::enable_shared_from_this<Fiber> {
    friend class Scheduler;

public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEPT,
    };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0, bool use_caller = false);
    ~Fiber();

    /**
     * @brief 重置协程函数, 并重置状态 INIT, TERM
     */
    void reset(std::function<void()> cb);

    /**
     * @brief 切换到当前协程执行
     */
    void swapIn();

    /**
     * @brief 切换到后台协程执行
     */
    void swapOut();

    // 相当于swapIn(), 但强行把当前协程置换成目标协程
    void call();

    void back();

    /**
     * @brief 返回协程id
     */
    const uint64_t getId() const { return m_id; }

    /**
     * @brief 返回协程状态
     */
    State getState() const { return m_state; }

    /**
     * @brief 设置当前协程
     */
    static void SetThis(Fiber* f);

    /**
     * @brief 返回当前协程
     *
     * @return static Fiber::ptr 当前协程智能指针
     */
    static Fiber::ptr GetThis();

    /**
     * @brief 协程切换到后台, 并且设置为Ready状态
     */
    static void YieldToReady();

    /**
     * @brief 协程切换到后台, 并且设置为Hold状态
     */
    static void YieldToHold();

    /**
     * @brief 统计总协程数
     *
     * @return uint64_t 总协程数
     */
    static uint64_t TotalFibers();

    static void MainFunc();
    static void CallerMainFunc();

    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;         // 协程号
    uint32_t m_stacksize = 0;  // 栈大小
    State m_state = INIT;      // 运行状态

    ucontext_t m_ctx;
    void* m_stack = nullptr;

    std::function<void()> m_cb;  // 协程执行函数
};

}  // namespace sylar
