#pragma once

#include <QString>
#include <QTextStream>

namespace Log {

enum class Level
{
    Debug,
    Info,
    Warning,
    Error
};

// 设置后所有日志(包括 Qt 自身的 qDebug/qWarning 等)镜像写入该文件,
// 传空路径关闭文件。无法打开文件时返回 false
bool setFile(const QString &path);

} // namespace Log

// 流式日志临时对象:Log::info() << "loaded" << 3 << "items";
// 按级别输出,析构时提交
class LogStream
{
public:
    explicit LogStream(Log::Level level);
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
    QString m_buffer;
    QTextStream m_stream;
};

namespace Log {

LogStream debug();
LogStream info();
LogStream warning();
LogStream error();

} // namespace Log
