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

#include <string>
#include <vector>

namespace sylar {

pid_t GetThreadId();

uint32_t GetFiberId();

void Backtrace(std::vector<std::string>& bt, int size, int skip = 1);

std::string BacktraceToString(int size, int skip = 2, const std::string& prefix = "");

}  // namespace sylar
