#include <iostream>
#include <memory>

#include "../src/log.h"
#include "../src/singleton.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger = std::make_shared<sylar::Logger>();
    logger->addAppender(std::make_shared<sylar::StdoutLogAppender>());

    sylar::FileLogAppender::ptr file_appender = std::make_shared<sylar::FileLogAppender>("./log.txt");
    sylar::LogFormatter::ptr fmt(std::make_shared<sylar::LogFormatter>("%d%T%p%T%m%n"));
    file_appender->setFormatter(fmt);
    file_appender->setLevel(sylar::LogLevel::ERROR);
    logger->addAppender(file_appender);

    std::cout << logger->getName() << ' ' << logger->getLevel() << std::endl;

    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";
    SYLAR_LOG_FMT_ERROR(logger, "test macro fmt error %s %d", "aa");

    auto l = sylar::LoggerMgr::GetInstance()->getLogger("xx");

    SYLAR_LOG_INFO(l) << "xxx";

    auto l_shared = sylar::LoggerMgrPtr::GetInstance()->getLogger("xx_shared");
    SYLAR_LOG_INFO(l_shared) << "xxx_shared";

    return 0;
}