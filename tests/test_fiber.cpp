#include <vector>

#include "src/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber() {
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    sylar::Fiber::YieldToHold();
}

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "main begin";
    {
        sylar::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "test begin";
        sylar::Fiber::ptr fiber = std::make_shared<sylar::Fiber>(run_in_fiber);
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "test after swapIn";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "test end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main end";
}

int main(int argc, char** argv) {
    sylar::Thread::SetName("main");

    int thread_num = 1;

    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < thread_num; ++i) {
        thrs.emplace_back(std::make_shared<sylar::Thread>(&test_fiber, "name_" + std::to_string(i)));
    }
    for (int i = 0; i < thread_num; ++i) {
        thrs[i]->join();
    }

    return 0;
}