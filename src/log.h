//===----------------------------------------------------------------------===//
//
//                         Sylar-Server
//
// log.h
//
// Identification: src/log.h
//
// Copyright (c) 2022, pyc
//
//===----------------------------------------------------------------------===//

#pragma once

#include <stdarg.h>

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "singleton.h"
#include "util.h"

/**
 * @brief 使用流式方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_LEVEL(logger, level)                                             \
    if (logger->getLevel() <= level)                                               \
    sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(__FILE__, __LINE__, 0,   \
                                                          sylar::GetThreadId(),    \
                                                          sylar::GetFiberId(),     \
                                                          time(0), logger, level)) \
        .getSS()

#define SYLAR_LOG_DEBUG(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::DEBUG)
#define SYLAR_LOG_INFO(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::INFO)
#define SYLAR_LOG_WARN(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::WARN)
#define SYLAR_LOG_ERROR(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::ERROR)
#define SYLAR_LOG_FATAL(logger) SYLAR_LOG_LEVEL(logger, sylar::LogLevel::FATAL)

/**
 * @brief 使用格式化方式将日志级别level的日志写入到logger
 */
#define SYLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                                                 \
    if (logger->getLevel() <= level)                                                                 \
    sylar::LogEventWrap(std::make_shared<sylar::LogEvent>(__FILE__, __LINE__, 0,                     \
                                                          sylar::GetThreadId(), sylar::GetFiberId(), \
                                                          time(0), logger, level))                   \
        .getEvent()                                                                                  \
        ->format(fmt, __VA_ARGS__)

#define SYLAR_LOG_FMT_DEBUG(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_INFO(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_WARN(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_ERROR(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SYLAR_LOG_FMT_FATAL(logger, fmt, ...) SYLAR_LOG_FMT_LEVEL(logger, sylar::LogLevel::FATAL, fmt, __VA_ARGS__)

/**
 * @brief 获取主日志器
 */
#define SYLAR_LOG_ROOT() sylar::LoggerMgr::GetInstance()->getRoot()

#define SYLAR_LOG_NAME(name) sylar::LoggerMgr::GetInstance()->getLogger(name)

namespace sylar {
class Logger;
class LoggerManger;

/**
 * @brief 日志级别
 */
class LogLevel {
public:
    enum Level {
        UNKNOW = 0,
        DEBUG = 1,
        INFO = 2,
        WARN = 3,
        ERROR = 4,
        FATAL = 5
    };

    static const char* ToString(Level level);
    static LogLevel::Level FromString(const std::string& str);
};

/**
 * @brief 日志事件
 */
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(const char* file, int32_t line, uint32_t elapse,
             uint32_t thread_id, uint32_t fiber_id, uint64_t time,
             std::shared_ptr<Logger> logger, LogLevel::Level level);

    const char* getFilename() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    const std::string getContent() const { return m_ss.str(); }
    std::stringstream& getSS() { return m_ss; }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }
    LogLevel::Level getLevel() const { return m_level; }

    /**
     * @brief 格式化写入日志内容
     */
    void format(const char* fmt, ...);
    void format(const char* fmt, va_list al);

private:
    const char* m_file = nullptr;      // 文件名
    int32_t m_line = 0;                // 行号
    uint32_t m_elapse = 0;             // 程序启动到现在的毫秒数
    uint32_t m_threadId = 0;           // 线程id
    uint32_t m_fiberId = 0;            // 协程id
    uint64_t m_time = 0;               // 时间戳
    std::stringstream m_ss;            // 日志内容流
    std::shared_ptr<Logger> m_logger;  // 日志器
    LogLevel::Level m_level;           // 日志等级
};

/**
 * @brief 日志事件包装器
 */
class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr event);
    ~LogEventWrap();

    LogEvent::ptr getEvent() const { return m_event; }
    std::stringstream& getSS();

private:
    LogEvent::ptr m_event;
};

// 日志格式器
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;

    LogFormatter(const std::string& pattern);
    std::string format(LogLevel::Level level, LogEvent::ptr event);

public:
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;

        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();  // pattern解析

    bool isError() const { return m_error; }
    const std::string getPattern() const { return m_pattern; }

private:
    std::string m_pattern;                 // 日志格式模板
    std::vector<FormatItem::ptr> m_items;  // 日志格式解析后格式
    bool m_error = false;                  // 是否有错误
};

// 日志输出地
class LogAppender {
    friend class Logger;

public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() {}
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;
    virtual std::string toYamlString() = 0;

    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }

    /**
     * @brief 获取日志级别
     */
    LogLevel::Level getLevel() const { return m_level; }

    /**
     * @brief 设置日志级别
     */
    void setLevel(LogLevel::Level val) { m_level = val; }

protected:
    LogLevel::Level m_level = LogLevel::DEBUG;
    LogFormatter::ptr m_formatter;
};

// 日志器
class Logger : public std::enable_shared_from_this<Logger> {
    friend class LoggerManager;

public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name = "root");
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);
    void clearAppenders();

    const std::string& getName() const { return m_name; }
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level level) { m_level = level; }
    void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }
    void setFormatter(const std::string& pattern);
    LogFormatter::ptr getFormatter() const { return m_formatter; }

    std::string toYamlString();

private:
    std::string m_name;                       // 日志名称
    LogLevel::Level m_level;                  // 日志级别
    std::list<LogAppender::ptr> m_appenders;  // 目标目录列表
    LogFormatter::ptr m_formatter;            // 日志格式器

    Logger::ptr m_root;  // 主日志器
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;
};
// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename);
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
    std::string toYamlString() override;

    // 重新打开文件, 文件打开成功返回true
    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;
};

/**
 * @brief 日志器管理类
 */
class LoggerManager {
public:
    LoggerManager();
    void init();
    Logger::ptr getLogger(const std::string& name);
    Logger::ptr getRoot() { return m_root; }

    std::string toYamlString();

private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
};

// 日志器管理类单例模式
typedef Singleton<LoggerManager> LoggerMgr;

typedef Singletonptr<LoggerManager> LoggerMgrPtr;

}  // namespace sylar
