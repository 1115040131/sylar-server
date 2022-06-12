#include <iostream>
#include <memory>

#include "../src/log.h"
#include "../src/util.h"

int main(int argc, char** argv) {
    sylar::Logger::ptr logger = std::make_shared<sylar::Logger>();
    logger->addAppender(std::make_shared<sylar::StdoutLogAppender>());
    std::cout << logger->getName() << ' ' << logger->getLevel() << std::endl;

    SYLAR_LOG_INFO(logger) << "test macro";
    SYLAR_LOG_ERROR(logger) << "test macro error";

    return 0;
}