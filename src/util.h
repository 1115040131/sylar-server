//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// util.h
//
// Identification: src/util.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once

#include <stdint.h>
#include <unistd.h>

namespace sylar {

pid_t GetThreadId();

uint32_t GetFiberId();

}  // namespace sylar
