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

#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

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

Logger::Logger(const std::string& name)
    : m_name(name),
      m_level(LogLevel::DEBUG) {
    m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T<%f:%l>%T%m%n"));
}

LogEventWrap::LogEventWrap(LogEvent::ptr event) : m_event(event) {}

LogEventWrap::~LogEventWrap() {
    m_event->getLogger()->log(m_event->getLevel(), m_event);
}

std::stringstream& LogEventWrap::getSS() {
    return m_event->getSS();
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        for (auto& i : m_appenders) {
            i->log(level, event);
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
    if (!appender->getFormatter()) {  // 用logger的样式初始化appender的样式
        appender->m_formatter = m_formatter;
    }
    m_appenders.emplace_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto iter = m_appenders.begin(); iter != m_appenders.end(); ++iter) {
        if (*iter == appender) {
            m_appenders.erase(iter);
            break;
        }
    }
}

void StdoutLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        std::cout << m_formatter->format(level, event);
    }
}

FileLogAppender::FileLogAppender(const std::string& filename) : m_filename(filename) {}

void FileLogAppender::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level >= m_level) {
        m_filestream << m_formatter->format(level, event);
    }
}

bool FileLogAppender::reopen() {
    if (m_filestream) {
        m_filestream.close();
    }
    m_filestream.open(m_filename);
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

}  // namespace sylar
