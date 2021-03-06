//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// util.cpp
//
// Identification: src/util.cpp
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#include "util.h"

#include <execinfo.h>
#include <stdint.h>
#include <syscall.h>
#include <unistd.h>

#include <sstream>

#include "fiber.h"
#include "log.h"

namespace sylar {

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return Fiber::GetFiberId();
}

void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
        free(array);
        array = nullptr;
        return;
    }

    for (size_t i = skip; i < s; ++i) {
        bt.emplace_back(strings[i]);
    }

    free(strings);
    strings = nullptr;
    free(array);
    array = nullptr;
}

std::string BacktraceToString(int size, int skip, const std::string& prefix) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

}  // namespace sylar
