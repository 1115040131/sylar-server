cmake_minimum_required(VERSION 3.0)
project(sylar)

# 设置__FILE__路径为相对路径
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17 -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(${PROJECT_SOURCE_DIR})
include_directories(/home/pyc/dev/yaml-cpp-yaml-cpp-0.7.0/include)

link_directories(/home/pyc/dev/yaml-cpp-yaml-cpp-0.7.0/build)

set (LIB_SRC 
     src/config.cpp
     src/fiber.cpp
     src/log.cpp
     src/thread.cpp
     src/util.cpp
     )

add_library(sylar SHARED ${LIB_SRC})
redefine_file_macro(sylar) #__FILE__

# add_library(sylar_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

set(LIBS
    sylar
    yaml-cpp
    pthread
    )

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_subdirectory(tests)
