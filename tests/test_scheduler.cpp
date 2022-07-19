#include "src/sylar.h"

static sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_fiber() {
    SYLAR_LOG_INFO(g_logger) << "test in fiber";
}

int main(int argc, char** argv) {
    SYLAR_LOG_INFO(g_logger) << "scheduler begin";
    sylar::Scheduler sc;
    sc.schedule(&test_fiber);
    sc.start();
    sc.stop();
    SYLAR_LOG_INFO(g_logger) << "scheduler end";

    return 0;
}