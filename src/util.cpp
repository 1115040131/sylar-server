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

#include <stdint.h>
#include <syscall.h>
#include <unistd.h>

namespace sylar {

pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

uint32_t GetFiberId() {
    return 0;
}

}  // namespace sylar
