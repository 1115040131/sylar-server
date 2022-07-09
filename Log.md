# Log

## 开发环境
Unbuntu 20.04
gcc 9.4
cmake

## 项目路径
bin — 二进制 \
build — 中间文件路径 \
cmake — cmake函数文件夹 \
CMakeLists.txt — cmake的定义文件 \
lib — 库的输出路径 \
Makefile \
src — 源代码路径 \
tests — 测试代码 

## 日志系统
1) Log4J \
Logger (定义日志类别) \
   | \
   |-------Formatter（日志格式）\
   | \
Appender（日志输出地方）

## 配置系统

Config --> Yaml 

yaml-cpp 

mkdir build && cd build && cmake .. && make install
```cpp
YAML::Node node =YAML::LoadFile(filename);
if (node.IsMap()) {
   for (auto& iter : node) {}
}
else if (node.IsSequence()) {
   for (size_t i = 0; i < node.size(); ++i) {}
}
else if (node.IsScalar()) {}
```
配置系统的原则，约定优于配置
```cpp
template<T, FromStr, ToStr>
class ConfigVar;

template<F, T>
class LexicalCast;

// 容器片特化，目前支持vector
// list, set, map, unordered_set, unordered_map
// map/unordered_set 支持key = std::string
// Config::Lookup(key) , key相同， 类型不同的，不会有报错。这个需要处理一下
```
自定义类型，需要实现sylar::LexicalCast,片特化
实现后，就可以支持Config解析自定义类型，自定义类型可以和常规stl容器一起使用。

配置的事件机制
当一个配置项发生修改的时候，可以反向通知对应的代码，回调  

# 日志系统整合配置系统
```yaml
logs: 
    - name: root
      level: (debug, info, warn, error, fatal)
      formatter: "%d%T%p%T%t%m%n"
      appenders:
          - type: (StdoutLogAppender, FileLogAppender)
            level: (debug, info, warn, error, fatal)
            file: /logs/xxx.log
```
```cpp
sylar::Logger g_logger = sylar::LoggerMgr::GetInstance()->getLogger(name);
SYLAR_LOG_INFO(g_logger) << "xxx log";
```
```cpp
static Logger::ptr g_log = SYLAR_LOG_NAME("system");
// 当logger的appenders为空时, 使用root写logger
```
定义LogDefine LogAppenderDefine，偏特化LexicalCast，

实现日志配置解析

## 线程库
Thread，Mutex，S/X-Lock

pthread pthread_create

互斥量 mutex

信号量 semaphore

和Log整合

文件输出周期性reopen

## 协程库封装
定义协程接口
ucontext_t
maro