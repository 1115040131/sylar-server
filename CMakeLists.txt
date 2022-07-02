cmake_minimum_required(VERSION 3.0)
project(sylar)

# 设置__FILE__路径为相对路径
include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -std=c++17 -rdynamic -O0 -g -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(/home/pyc/dev/yaml-cpp-yaml-cpp-0.7.0/include)

link_directories(/home/pyc/dev/yaml-cpp-yaml-cpp-0.7.0/build)

set (LIB_SRC 
    src/log.cpp
    src/util.cpp
    src/config.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
redefine_file_macro(sylar) #__FILE__
# add_library(sylar_static STATIC ${LIB_SRC})
# SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

add_executable(test tests/test.cpp)
add_dependencies(test sylar)
redefine_file_macro(test) #__FILE__
target_link_libraries(test sylar yaml-cpp)

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config sylar)
redefine_file_macro(test) #__FILE__
target_link_libraries(test_config sylar yaml-cpp)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
