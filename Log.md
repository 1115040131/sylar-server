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

// 容器偏特化，目前支持vector
```