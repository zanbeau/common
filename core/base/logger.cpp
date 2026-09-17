#include "logger.h"

#include <QDateTime>
#include <QFile>
#include <QMutex>

namespace {

QMutex g_mutex;
QFile g_file;
QtMessageHandler g_previous = nullptr;

QString levelName(QtMsgType type)
{
    switch(type)
    {
        case QtDebugMsg:    return QStringLiteral("DEBUG");
        case QtInfoMsg:     return QStringLiteral("INFO");
        case QtWarningMsg:  return QStringLiteral("WARN");
        case QtCriticalMsg: return QStringLiteral("ERROR");
        case QtFatalMsg:    return QStringLiteral("FATAL");
    }
    return QStringLiteral("INFO");
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker lock(&g_mutex);
    if(g_file.isOpen())
    {
        const QString line = QStringLiteral("[%1] [%2] %3\n")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                 levelName(type), message);
        g_file.write(line.toUtf8());
        g_file.flush();
    }
    if(g_previous)
    {
        g_previous(type, context, message);
    }
}

} // namespace

LogStream::LogStream(Log::Level level)
    : m_level(level),
      m_stream(&m_buffer)
{
}

LogStream::~LogStream()
{
    m_stream.flush();
    const QByteArray utf8 = m_buffer.toUtf8();
    switch(m_level)
    {
        case Log::Level::Debug:    qDebug("%s", utf8.constData()); break;
        case Log::Level::Info:     qInfo("%s", utf8.constData()); break;
        case Log::Level::Warning:  qWarning("%s", utf8.constData()); break;
        case Log::Level::Error:    qCritical("%s", utf8.constData()); break;
    }
}

namespace Log {

LogStream debug()   { return LogStream(Level::Debug); }
LogStream info()    { return LogStream(Level::Info); }
LogStream warning() { return LogStream(Level::Warning); }
LogStream error()   { return LogStream(Level::Error); }

bool setFile(const QString &path)
{
    QMutexLocker lock(&g_mutex);
    g_file.close();
    g_file.setFileName(path);
    if(path.isEmpty())
    {
        return true;
    }
    if(!g_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        return false;
    }
    if(!g_previous)
    {
        g_previous = qInstallMessageHandler(messageHandler);
    }
    return true;
}

} // namespace Log
