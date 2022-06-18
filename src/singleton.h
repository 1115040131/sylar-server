//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// singleton.h
//
// Identification: src/singleton.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once
#include <memory>

namespace sylar {

template <class T, class X = void, int N = 0>
class Singleton {
public:
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template <class T, class X = void, int N = 0>
class Singletonptr {
public:
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v = std::make_shared<T>();
        return v;
    }
};

}  // namespace sylar
