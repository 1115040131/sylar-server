//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// log.cpp
//
// Identification: src/log.cpp
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#include "log.h"

#include <stdarg.h>

#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "config.h"

namespace sylar {

const char* LogLevel::ToString(LogLevel::Level level) {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
            break;
        case LogLevel::INFO:
            return "INFO";
            break;
        case LogLevel::WARN:
            return "WARN";
            break;
        case LogLevel::ERROR:
            return "ERROR";
            break;
        case LogLevel::FATAL:
            return "FATAL";
            break;
        default:
            return "UNKNOW";
    }
    return "UNKNOW";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
#define XX(level, v)            \
    if (str == #v) {            \
        return LogLevel::level; \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOW;
#undef XX
}

class MessageFormatItem : public LogFormatter::FormatItem {
public:
    MessageFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};
class LevelFormatItem : public LogFormatter::FormatItem {
public:
    LevelFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << LogLevel::ToString(level);
    }
};
class ElapseFormatItem : public LogFormatter::FormatItem {
public:
    ElapseFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getElapse();
    }
};
class NameFormatItem : public LogFormatter::FormatItem {
public:
    NameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLogger()->getName();
    }
};
class ThreadIdFormatItem : public LogFormatter::FormatItem {
public:
    ThreadIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getThreadId();
    }
};
class FiberIdFormatItem : public LogFormatter::FormatItem {
public:
    FiberIdFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFiberId();
    }
};
class TimeFormatItem : public LogFormatter::FormatItem {
public:
    TimeFormatItem(const std::string& format)
        : m_format(format) {
        if (m_format.empty()) {
            m_format = "%Y-%m-%d %H:%M:%S";
        }
    }
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        struct tm tm;
        time_t time = event->getTime();
        localtime_r(&time, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), m_format.c_str(), &tm);
        os << buf;
    }

private:
    std::string m_format;
};
class FilenameFormatItem : public LogFormatter::FormatItem {
public:
    FilenameFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getFilename();
    }
};
class LineFormatItem : public LogFormatter::FormatItem {
public:
    LineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << event->getLine();
    }
};
class NewLineFormatItem : public LogFormatter::FormatItem {
public:
    NewLineFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << std::endl;
    }
};
class StringFormatItem : public LogFormatter::FormatItem {
public:
    StringFormatItem(const std::string& str) : m_string(str) {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << m_string;
    }

private:
    std::string m_string;
};
class TabFormatItem : public LogFormatter::FormatItem {
public:
    TabFormatItem(const std::string& str = "") {}
    void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) override {
        os << "\t";
    }
};

LogEvent::LogEvent(const char* file, int32_t line, uint32_t elapse,
                   uint32_t thread_id, uint32_t fiber_id, uint64_t time,
                   std::shared_ptr<Logger> logger, LogLevel::Level level)
    : m_file(file),
      m_line(line),
      m_elapse(elapse),
      m_threadId(thread_id),
      m_fiberId(fiber_id),
      m_time(time),
      m_logger(logger),
      m_level(level) {}

LogEventWrap::LogEventWrap(LogEvent::ptr event) : m_event(event) {}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

void LogEvent::format(const char* fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void LogEvent::format(const char* fmt, va_list al) {
    char* buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

LogFormatter::ptr LogAppender::getFormatter() {
    MutexType::Lock lock(m_mutex);

    return m_formatter;
}

void LogAppender::setFormatter(LogFormatter::ptr formatter) {
    MutexType::Lock lock(m_mutex);

    m_formatter = formatter;
    if (m_formatter) {
        m_has_formatter = true;
    } else {
        m_has_formatter = false;
    }
}

Logger::Logger(const std::string& name)
    : m_name(name),
      m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%n"));
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        // auto self = shared_from_this();

        MutexType::Lock lock(m_mutex);

        if (!m_appenders.empty()) {
            for (auto& appender : m_appenders) {
                appender->log(level, event);
            }
        } else if (m_root) {
            m_root->log(level, event);
        }
    }
}

void Logger::debug(LogEvent::ptr event) {
    log(LogLevel::Level::DEBUG, event);
}

void Logger::info(LogEvent::ptr event) {
    log(LogLevel::Level::INFO, event);
}

void Logger::warn(LogEvent::ptr event) {
    log(LogLevel::Level::WARN, event);
}

void Logger::error(LogEvent::ptr event) {
    log(LogLevel::Level::ERROR, event);
}

void Logger::fatal(LogEvent::ptr event) {
    log(LogLevel::Level::FATAL, event);
}

void Logger::addAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);
    // appender没有自己的样式时, 用logger的样式初始化, 保持m_has_formatter不变
    if (!appender->getFormatter()) {
        MutexType::Lock lock(appender->m_mutex);

        appender->m_formatter = m_formatter;
    }
    m_appenders.emplace_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    MutexType::Lock lock(m_mutex);

    for (auto iter = m_appenders.begin(); iter != m_appenders.end(); ++iter) {
        if (*iter == appender) {
            m_appenders.erase(iter);
            break;
        }
    }
}

void Logger::clearAppenders() {
    MutexType::Lock lock(m_mutex);

    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr formatter) {
    MutexType::Lock lock(m_mutex);

    m_formatter = formatter;
    for (auto& appender : m_appenders) {
        MutexType::Lock lock(appender->m_mutex);

        if (!appender->m_has_formatter) {
            appender->m_formatter = formatter;
        }
    }
}

void Logger::setFormatter(const std::string& pattern) {
    LogFormatter::ptr formatter = std::make_shared<LogFormatter>(pattern);
    if (formatter->isError()) {
        std::cout << "Logger setFormatter name=" << m_name
                  << " value=" << pattern << " invalid foramtter" << std::endl;
        return;
    }

    setFormatter(formatter);
}

std::string Logger::toYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    if (m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    for (auto& appender : m_appenders) {
        node["appenders"].push_back(YAML::Load(appender->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        MutexType::Lock lock(m_mutex);

        std::cout << m_formatter->format(level, event);
    }
}

std::string StdoutLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    node["type"] = "StdoutLogAppender";

    if (m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if (m_has_formatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) {
    reopen();
}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        uint64_t now = time(0);
        if (now != m_last_time) {
            reopen();
            m_last_time = now;
        }

        MutexType::Lock lock(m_mutex);

        m_filestream << m_formatter->format(level, event);
    }
}

std::string FileLogAppender::toYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    node["type"] = "FileLogAppender";
    node["file"] = m_filename;
    if (m_level != LogLevel::UNKNOW) {
        node["level"] = LogLevel::ToString(m_level);
    }
    if (m_has_formatter && m_formatter) {
        node["formatter"] = m_formatter->getPattern();
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

bool FileLogAppender::reopen() {
    MutexType::Lock lock(m_mutex);

    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename, std::ios::app);
    return !!m_filestream;
}

LogFormatter::LogFormatter(const std::string& pattern) : m_pattern(pattern) {
    init();
}

std::string LogFormatter::format(LogLevel::Level level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto& i : m_items) {
        i->format(ss, level, event);
    }
    return ss.str();
}

void LogFormatter::init() {
    // %xxx 纯文本
    // %xxx{xxx} 带格式的文本
    // %str{fmt} example: %d{20.32}
    // %m -- 消息体
    // %r -- 启动后的时间
    // %c -- 日志名称
    // %t -- 线程id
    // %n -- 回车换行
    // %d -- 时间
    // %f -- 文件名
    // %l -- 行号
    // str, format, type
    std::vector<std::tuple<std::string, std::string, int>> vec;
    std::string nstr;
    for (size_t i = 0; i < m_pattern.size(); ++i) {
        if (m_pattern[i] != '%') {
            nstr.append(1, m_pattern[i]);
            continue;
        }

        if ((i + 1) < m_pattern.size()) {
            if (m_pattern[i + 1] == '%') {
                nstr.append(1, '%');
                continue;
            }
        }

        size_t n = i + 1;
        int fmt_status = 0;
        size_t fmt_begin = 0;

        std::string str;
        std::string fmt;
        while (n < m_pattern.size()) {
            if (!fmt_status && (!isalpha(m_pattern[n]) && m_pattern[n] != '{' && m_pattern[n] != '}')) {
                str = m_pattern.substr(i + 1, n - i - 1);
                break;
            }
            if (fmt_status == 0) {
                if (m_pattern[n] == '{') {
                    str = m_pattern.substr(i + 1, n - i - 1);
                    fmt_status = 1;
                    fmt_begin = n;
                    ++n;
                    continue;
                }
            } else if (fmt_status == 1) {
                if (m_pattern[n] == '}') {
                    fmt = m_pattern.substr(fmt_begin + 1, n - fmt_begin - 1);
                    fmt_status = 0;
                    ++n;
                    break;
                }
            }
            ++n;
            if (n == m_pattern.size()) {
                if (str.empty()) {
                    str = m_pattern.substr(i + 1);
                }
            }
        }

        if (fmt_status == 0) {
            if (!nstr.empty()) {
                vec.emplace_back(nstr, std::string(), 0);
                nstr.clear();
            }
            vec.emplace_back(str, fmt, 1);
            i = n - 1;
        } else if (fmt_status == 1) {
            std::cout << "pattern parse error: " << m_pattern << " - " << m_pattern.substr(i) << std::endl;
            m_error = true;
            vec.emplace_back("<<pattern_error>>", fmt, 0);
        }
    }
    if (!nstr.empty()) {
        vec.emplace_back(nstr, std::string(), 0);
    }
    static std::map<std::string, std::function<FormatItem::ptr(const std::string& fmt)>> s_format_items = {
        {"m", [](const std::string& fmt) { return std::make_shared<MessageFormatItem>(fmt); }},
        {"p", [](const std::string& fmt) { return std::make_shared<LevelFormatItem>(fmt); }},
        {"r", [](const std::string& fmt) { return std::make_shared<ElapseFormatItem>(fmt); }},
        {"c", [](const std::string& fmt) { return std::make_shared<NameFormatItem>(fmt); }},
        {"t", [](const std::string& fmt) { return std::make_shared<ThreadIdFormatItem>(fmt); }},
        {"F", [](const std::string& fmt) { return std::make_shared<FiberIdFormatItem>(fmt); }},
        {"d", [](const std::string& fmt) { return std::make_shared<TimeFormatItem>(fmt); }},
        {"f", [](const std::string& fmt) { return std::make_shared<FilenameFormatItem>(fmt); }},
        {"l", [](const std::string& fmt) { return std::make_shared<LineFormatItem>(fmt); }},
        {"n", [](const std::string& fmt) { return std::make_shared<NewLineFormatItem>(fmt); }},
        {"T", [](const std::string& fmt) { return std::make_shared<TabFormatItem>(fmt); }},
    };

    for (auto& i : vec) {
        if (std::get<2>(i) == 0) {
            m_items.emplace_back(std::make_shared<StringFormatItem>(std::get<0>(i)));
        } else {
            auto iter = s_format_items.find(std::get<0>(i));
            if (iter == s_format_items.end()) {
                m_items.emplace_back(std::make_shared<StringFormatItem>("<<error_format %" + std::get<0>(i) + ">>"));
                m_error = true;
            } else {
                m_items.emplace_back(iter->second(std::get<1>(i)));
            }
        }
        // std::cout << "(" << std::get<0>(i) << ") - (" << std::get<1>(i) << ") - (" << std::get<2>(i) << ")" << std::endl;
    }
}

LoggerManager::LoggerManager() {
    m_root.reset(new Logger());
    m_root->addAppender(std::make_shared<StdoutLogAppender>());

    m_loggers[m_root->m_name] = m_root;
}

Logger::ptr LoggerManager::getLogger(const std::string& name) {
    MutexType::Lock lock(m_mutex);

    auto iter = m_loggers.find(name);
    if (iter != m_loggers.end()) {
        return iter->second;
    }
    // 如果没有找到该logger, 则创建
    Logger::ptr logger = std::make_shared<Logger>(name);
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}

std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    for (auto& [name, logger] : m_loggers) {
        node.push_back(YAML::Load(logger->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}

struct LogAppenderDefine {
    int type = 0;  // 1: File, 2: Stdout
    LogLevel::Level level = LogLevel::UNKNOW;
    std::string formatter;
    std::string file;

    bool operator==(const LogAppenderDefine& oth) const {
        return type == oth.type &&
               level == oth.level &&
               formatter == oth.formatter &&
               file == oth.file;
    }
};

struct LogDefine {
    std::string name;                          // 日志名称
    LogLevel::Level level = LogLevel::UNKNOW;  // 日志级别
    std::vector<LogAppenderDefine> appenders;  // 目标目录列表
    std::string formatter;                     // 日志格式器

    bool operator==(const LogDefine& oth) const {
        return name == oth.name &&
               level == oth.level &&
               formatter == oth.formatter &&
               appenders == oth.appenders;
    }

    bool operator<(const LogDefine& oth) const {
        return name < oth.name;
    }
};

template <>
class LexicalCast<std::string, LogDefine> {
public:
    LogDefine operator()(const std::string& str) {
        YAML::Node node = YAML::Load(str);
        LogDefine log_define;
        if (!node["name"].IsDefined()) {
            std::cout << "log config error: name is null, " << node << std::endl;
            throw std::logic_error("log config name is null");
        }
        log_define.name = node["name"].as<std::string>();
        log_define.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
        if (node["formatter"].IsDefined()) {
            log_define.formatter = node["formatter"].as<std::string>();
        }
        if (node["appenders"].IsDefined()) {
            for (size_t i = 0; i < node["appenders"].size(); ++i) {
                auto appender_node = node["appenders"][i];
                if (!appender_node["type"].IsDefined()) {
                    std::cout << "log config error: appender type is null, " << appender_node << std::endl;
                    continue;
                }
                std::string type = appender_node["type"].as<std::string>();
                LogAppenderDefine appender_define;
                if (type == "FileLogAppender") {
                    appender_define.type = 1;
                    if (!appender_node["file"].IsDefined()) {
                        std::cout << "log config error: fileappender file is null, " << appender_node << std::endl;
                        continue;
                    }
                    appender_define.file = appender_node["file"].as<std::string>();
                    if (appender_node["formatter"].IsDefined()) {
                        appender_define.formatter = appender_node["formatter"].as<std::string>();
                    }
                } else if (type == "StdoutLogAppender") {
                    appender_define.type = 2;
                } else {
                    std::cout << "log config error: appender type is invalid, " << appender_node << std::endl;
                    continue;
                }
                log_define.appenders.emplace_back(appender_define);
            }
        }
        return log_define;
    }
};

template <>
class LexicalCast<LogDefine, std::string> {
public:
    std::string operator()(const LogDefine& log_define) {
        YAML::Node node;
        node["name"] = log_define.name;
        if (log_define.level != LogLevel::UNKNOW) {
            node["level"] = LogLevel::ToString(log_define.level);
        }
        if (!log_define.formatter.empty()) {
            node["formatter"] = log_define.formatter;
        }
        for (auto& appender_define : log_define.appenders) {
            YAML::Node appender_node;
            if (appender_define.type == 1) {
                appender_node["type"] = "FileLogAppender";
                appender_node["file"] = appender_define.file;
            } else if (appender_define.type == 2) {
                appender_node["type"] = "StdoutLogAppender";
            }
            if (appender_define.level != LogLevel::UNKNOW) {
                appender_node = LogLevel::ToString(appender_define.level);
            }
            if (!appender_define.formatter.empty()) {
                appender_node["formatter"] = appender_define.formatter;
            }
            node["appenders"].push_back(appender_node);
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

sylar::ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
    sylar::Config::Lookup("logs", std::set<LogDefine>(), "logs config");

struct LogIniter {
    LogIniter() {
        g_log_defines->addListener(0xF1E231, [](const std::set<LogDefine>& old_value,
                                                const std::set<LogDefine>& new_value) {
            SYLAR_LOG_INFO(SYLAR_LOG_ROOT()) << "on_logger_conf_changed";
            for (auto& log_define : new_value) {
                auto iter = old_value.find(log_define);
                Logger::ptr logger;
                if (iter == old_value.end()) {
                    // 新增logger
                    logger = SYLAR_LOG_NAME(log_define.name);
                } else {
                    if (!(log_define == *iter)) {
                        // 修改logger
                        logger = SYLAR_LOG_NAME(log_define.name);
                    } else {
                        // 完全相同不执行操作
                        continue;
                    }
                }
                logger->setLevel(log_define.level);
                if (!log_define.formatter.empty()) {
                    // 如果log_define.formatter格式有问题, logger.formatter格式保持默认
                    logger->setFormatter(log_define.formatter);
                }
                logger->clearAppenders();
                for (auto& appender_define : log_define.appenders) {
                    LogAppender::ptr appender;
                    if (appender_define.type == 1) {
                        appender.reset(new FileLogAppender(appender_define.file));
                    } else if (appender_define.type == 2) {
                        appender.reset(new StdoutLogAppender());
                    }
                    appender->setLevel(appender_define.level);
                    if (!appender_define.formatter.empty()) {
                        LogFormatter::ptr fmt = std::make_shared<LogFormatter>(appender_define.formatter);
                        if (!fmt->isError()) {
                            appender->setFormatter(fmt);
                        } else {
                            std::cout << "logger name=" << log_define.name << " appender type=" << appender_define.type
                                      << " formatter=" << appender_define.formatter << " is invalid" << std::endl;
                        }
                    }
                    logger->addAppender(appender);
                }
            }

            for (auto& log_define : old_value) {
                auto iter = new_value.find(log_define);
                if (iter == new_value.end()) {
                    // 删除logger
                    auto logger = SYLAR_LOG_NAME(log_define.name);
                    logger->setLevel((LogLevel::Level)100);
                    logger->clearAppenders();
                }
            }
        });
    }
};
// 全局对象的构造函数在main函数之前
static LogIniter __log_init;

}  // namespace sylar
