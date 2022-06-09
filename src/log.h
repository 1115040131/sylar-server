#pragma once

#include <fstream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace sylar {

class Logger;

/**
 * @brief 日志级别
 */
class LogLevel {
public:
    enum Level {
        DEBUG = 1,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    static const char* ToString(Level level);
};

/**
 * @brief 日志事件
 */
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent() = default;
    const char* getFilename() const { return m_file; }
    int32_t getLine() const { return m_line; }
    uint32_t getElapse() const { return m_elapse; }
    uint32_t getThreadId() const { return m_threadId; }
    uint32_t getFiberId() const { return m_fiberId; }
    uint64_t getTime() const { return m_time; }
    const std::string& getContent() const { return m_content; }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }

private:
    const char* m_file = nullptr;  // 文件名
    int32_t m_line = 0;            // 行号
    uint32_t m_elapse = 0;         // 程序启动到现在的毫秒数
    uint32_t m_threadId = 0;       // 线程id
    uint32_t m_fiberId = 0;        // 协程id
    uint64_t m_time = 0;           // 时间
    std::string m_content;         // 消息

    std::shared_ptr<Logger> m_logger;  // 日志器
    LogLevel::Level m_level;           // 日志等级
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

        FormatItem(const std::string& fmt = "") {}
        virtual ~FormatItem() {}
        virtual void format(std::ostream& os, LogLevel::Level level, LogEvent::ptr event) = 0;
    };

    void init();  // pattern解析

private:
    std::string m_pattern;                 // 日志格式模板
    std::vector<FormatItem::ptr> m_items;  // 日志格式解析后格式
    bool m_error = false;                  // 是否有错误
};

// 日志输出地
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() {}
    virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

    LogFormatter::ptr getFormatter() const { return m_formatter; }
    void setFormatter(LogFormatter::ptr formatter) { m_formatter = formatter; }

protected:
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
};

// 日志器
class Logger {
public:
    typedef std::shared_ptr<Logger> ptr;

    Logger(const std::string& name);
    void log(LogLevel::Level level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);
    void delAppender(LogAppender::ptr appender);

    const std::string& getName() const { return m_name; }
    LogLevel::Level getLevel() const { return m_level; }
    void setLevel(LogLevel::Level level) { m_level = level; }

private:
    std::string m_name;                       // 日志名称
    LogLevel::Level m_level;                  // 日志级别
    std::list<LogAppender::ptr> m_appenders;  // Appender列表
};

// 输出到控制台的Appender
class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;
};
// 输出到文件的Appender
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename);
    virtual void log(LogLevel::Level level, LogEvent::ptr event) override;

    // 重新打开文件, 文件打开成功返回true
    bool reopen();

private:
    std::string m_filename;
    std::ofstream m_filestream;
};

}  // namespace sylar
