#include <iostream>

#include "src/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
sylar::RWMutex s_mutex;

void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName()
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();

    for (int i = 0; i < 100000; ++i) {
        sylar::RWMutex::WriteLock lock(s_mutex);
        ++count;
    }
}

void fun2() {
}

int main(int argc, char** argv) {
    int thread_num = 5;

    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    std::vector<sylar::Thread::ptr> thrs;
    thrs.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        sylar::Thread::ptr thr = std::make_shared<sylar::Thread>(&fun1, "name_" + std::to_string(i));
        thrs.push_back(thr);
    }

    for (int i = 0; i < thread_num; ++i) {
        thrs[i]->join();
    }
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    SYLAR_LOG_INFO(g_logger) << "count=" << count;

    return 0;
}