#include <iostream>

#include "src/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

int count = 0;
// sylar::RWMutex s_mutex;
sylar::Mutex s_mutex;

void fun1() {
    SYLAR_LOG_INFO(g_logger) << "name: " << sylar::Thread::GetName()
                             << " this.name: " << sylar::Thread::GetThis()->getName()
                             << " id: " << sylar::GetThreadId()
                             << " this.id: " << sylar::Thread::GetThis()->getId();

    for (int i = 0; i < 100000; ++i) {
        // sylar::RWMutex::WriteLock lock(s_mutex);
        sylar::Mutex::Lock lock(s_mutex);
        ++count;
    }
}

void fun2() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
    }
}

void fun3() {
    while (true) {
        SYLAR_LOG_INFO(g_logger) << "========================================================";
    }
}

void test_thread() {
    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    int thread_num = 5;
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
}

void test_mutex() {
    YAML::Node root = YAML::LoadFile("/home/pyc/dev/sylar/bin/conf/log2.yml");
    sylar::Config::LoadFromYaml(root);

    SYLAR_LOG_INFO(g_logger) << "thread test begin";
    int thread_num = 2;
    std::vector<sylar::Thread::ptr> thrs;
    thrs.reserve(thread_num);
    for (int i = 0; i < thread_num; ++i) {
        sylar::Thread::ptr thr1 = std::make_shared<sylar::Thread>(&fun2, "name_" + std::to_string(i * 2));
        sylar::Thread::ptr thr2 = std::make_shared<sylar::Thread>(&fun3, "name_" + std::to_string(i * 2 + 1));
        thrs.push_back(thr1);
        thrs.push_back(thr2);
    }

    for (size_t i = 0; i < thrs.size(); ++i) {
        thrs[i]->join();
    }
    SYLAR_LOG_INFO(g_logger) << "thread test end";
    SYLAR_LOG_INFO(g_logger) << "count=" << count;
}

int main(int argc, char** argv) {
    test_thread();
    // test_mutex();

    return 0;
}