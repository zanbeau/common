#pragma once

#include <QDateTime>
#include <QString>
#include <QTextStream>

namespace Log {

enum class Level
{
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
};

// 设置日志文件基准路径(如 "logs/app.log"):实际写入 <目录>/<基准名>_<日期>_<序号>.log,
// 跨天或单文件超过上限(默认 5MB)时滚动到新文件;打开时顺带清理同目录同基准名、
// 超过保质期(默认 7 天,0 = 立即全部过期,<0 = 不清理)的旧 .log。
// 设置后所有日志(包括 Qt 自身的 qDebug/qWarning 等)镜像写入该文件,
// 传空路径关闭文件。目录创建失败或文件无法打开时返回 false
bool setFile(const QString &path);

// 滚动的单文件大小上限,默认 5MB(借自 TTK 的 maxSize)
void setMaxSize(qint64 size);

// 日志文件保质期(天),默认 7 天(借自 TTK 的 expire)
void setExpireDays(int days);

} // namespace Log

// 流式日志临时对象:析构时按级别提交到 Qt 消息管线。
// file/line 由下方 LOG_X 宏在调用点捕获,自由函数(Log::info() 等)不带调用点信息
class LogStream
{
public:
    explicit LogStream(Log::Level level, const char *file = nullptr, int line = 0);
    ~LogStream();

    template<typename T>
    LogStream &operator<<(const T &value)
    {
        if(!m_buffer.isEmpty())
        {
            m_stream << ' '; // 与 QDebug 一致,条目之间用空格分隔
        }
        m_stream << value;
        return *this;
    }

private:
    Log::Level m_level;
    QString m_file;
    int m_line;
    QString m_buffer;
    QTextStream m_stream;
};

namespace Log {

LogStream trace();   // 走独立 category,镜像文件里才能与 DEBUG 区分开
LogStream debug();
LogStream info();
LogStream warning();
LogStream error();
LogStream fatal();   // qFatal 语义:提交后程序中止,这是它与 error 的区别所在

} // namespace Log

// 调用点宏族:镜像文件里带 [文件名(行号)] 前缀。流式参数用 << 连接,
// 顶层出现逗号需自行加括号(宏参数切分,与 TTK 相同的限制)。
// 例:LOG_INFO << "loaded" << 3 << "items";
#define LOG_TRACE   LogStream(Log::Level::Trace,   __FILE__, __LINE__)
#define LOG_DEBUG   LogStream(Log::Level::Debug,   __FILE__, __LINE__)
#define LOG_INFO    LogStream(Log::Level::Info,    __FILE__, __LINE__)
#define LOG_WARNING LogStream(Log::Level::Warning, __FILE__, __LINE__)
#define LOG_ERROR   LogStream(Log::Level::Error,   __FILE__, __LINE__)
#define LOG_FATAL   LogStream(Log::Level::Fatal,   __FILE__, __LINE__)

// 节流变体(借自 TTK 的 _ONCE/_COND/_COUNT/_PERIOD 宏族)。
// 内部是函数级 static,计数/计时非原子,多线程下偶发多打一条无妨
#define LOG_ONCE_IMPL(log) \
    do { static bool logged = false; if(!logged) { logged = true; log; } } while(0)
#define LOG_COND_IMPL(cond, log) \
    do { if((cond)) { log; } } while(0)
// 每 count 次调用输出一次(TTK 同款语义)
#define LOG_COUNT_IMPL(count, log) \
    do { static int hits = 1; if((count) > 0 && ++hits > (count)) { hits = 1; log; } } while(0)
// 每 seconds 秒最多输出一次
#define LOG_PERIOD_IMPL(seconds, log) \
    do { static qint64 last = 0; const qint64 now = QDateTime::currentMSecsSinceEpoch(); \
         if(last + (seconds) * 1000 <= now || now < last) { last = now; log; } } while(0)

#define LOG_TRACE_ONCE(...)   LOG_ONCE_IMPL(LOG_TRACE << __VA_ARGS__)
#define LOG_DEBUG_ONCE(...)   LOG_ONCE_IMPL(LOG_DEBUG << __VA_ARGS__)
#define LOG_INFO_ONCE(...)    LOG_ONCE_IMPL(LOG_INFO << __VA_ARGS__)
#define LOG_WARNING_ONCE(...) LOG_ONCE_IMPL(LOG_WARNING << __VA_ARGS__)
#define LOG_ERROR_ONCE(...)   LOG_ONCE_IMPL(LOG_ERROR << __VA_ARGS__)
#define LOG_FATAL_ONCE(...)   LOG_ONCE_IMPL(LOG_FATAL << __VA_ARGS__)

#define LOG_TRACE_COND(cond, ...)   LOG_COND_IMPL(cond, LOG_TRACE << __VA_ARGS__)
#define LOG_DEBUG_COND(cond, ...)   LOG_COND_IMPL(cond, LOG_DEBUG << __VA_ARGS__)
#define LOG_INFO_COND(cond, ...)    LOG_COND_IMPL(cond, LOG_INFO << __VA_ARGS__)
#define LOG_WARNING_COND(cond, ...) LOG_COND_IMPL(cond, LOG_WARNING << __VA_ARGS__)
#define LOG_ERROR_COND(cond, ...)   LOG_COND_IMPL(cond, LOG_ERROR << __VA_ARGS__)
#define LOG_FATAL_COND(cond, ...)   LOG_COND_IMPL(cond, LOG_FATAL << __VA_ARGS__)

#define LOG_TRACE_COUNT(count, ...)   LOG_COUNT_IMPL(count, LOG_TRACE << __VA_ARGS__)
#define LOG_DEBUG_COUNT(count, ...)   LOG_COUNT_IMPL(count, LOG_DEBUG << __VA_ARGS__)
#define LOG_INFO_COUNT(count, ...)    LOG_COUNT_IMPL(count, LOG_INFO << __VA_ARGS__)
#define LOG_WARNING_COUNT(count, ...) LOG_COUNT_IMPL(count, LOG_WARNING << __VA_ARGS__)
#define LOG_ERROR_COUNT(count, ...)   LOG_COUNT_IMPL(count, LOG_ERROR << __VA_ARGS__)
#define LOG_FATAL_COUNT(count, ...)   LOG_COUNT_IMPL(count, LOG_FATAL << __VA_ARGS__)

#define LOG_TRACE_PERIOD(seconds, ...)   LOG_PERIOD_IMPL(seconds, LOG_TRACE << __VA_ARGS__)
#define LOG_DEBUG_PERIOD(seconds, ...)   LOG_PERIOD_IMPL(seconds, LOG_DEBUG << __VA_ARGS__)
#define LOG_INFO_PERIOD(seconds, ...)    LOG_PERIOD_IMPL(seconds, LOG_INFO << __VA_ARGS__)
#define LOG_WARNING_PERIOD(seconds, ...) LOG_PERIOD_IMPL(seconds, LOG_WARNING << __VA_ARGS__)
#define LOG_ERROR_PERIOD(seconds, ...)   LOG_PERIOD_IMPL(seconds, LOG_ERROR << __VA_ARGS__)
#define LOG_FATAL_PERIOD(seconds, ...)   LOG_PERIOD_IMPL(seconds, LOG_FATAL << __VA_ARGS__)
