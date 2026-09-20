#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutex>
#include <QLoggingCategory>

namespace {

// trace 级别走独立 category:Qt 消息管线只有 5 个类型,
// 镜像文件里靠 category 名把 TRACE 从 DEBUG 里区分出来
Q_LOGGING_CATEGORY(lcTrace, "canfan.trace")

QMutex g_mutex;
QFile g_file;
QtMessageHandler g_previous = nullptr;

// 自家转发的消息 context 也指向 logger.cpp,用来和真正的调用方 context 区分
const char g_selfFile[] = __FILE__;

qint64 g_maxSize = 5 * 1024 * 1024;
qint64 g_expireDays = 7;

// 滚动状态:目录 + 基准名 + 当前文件对应的日期
QString g_dir, g_base, g_date;

QString levelName(QtMsgType type, const QMessageLogContext &context)
{
    if(type == QtDebugMsg && context.category && qstrcmp(context.category, "canfan.trace") == 0)
    {
        return QStringLiteral("TRACE");
    }
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

// __FILE__ 在 MSVC+Ninja 下通常是绝对路径,日志里只保留文件名
QString shortFile(const char *path)
{
    QString name = QString::fromUtf8(path);
    const auto slash = qMax(name.lastIndexOf(QLatin1Char('/')), name.lastIndexOf(QLatin1Char('\\')));
    return slash < 0 ? name : name.mid(slash + 1);
}

// 打开(或滚动到)当前日期的日志文件:<基准名>_<日期>_<序号>.log,
// 沿用 TTK 的取号方式:从 1 开始找第一个没超过上限的序号
void openFileLocked()
{
    g_date = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
    const QString fileBase = g_dir + QLatin1Char('/') + g_base + QLatin1Char('_') + g_date;
    int index = 1;
    do
    {
        g_file.setFileName(fileBase + QStringLiteral("_%1.log").arg(index++));
    }
    while(g_file.size() >= g_maxSize);
    g_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
}

// 清理过期旧日志(TTK removeFiles 的前缀限定版:只删同基准名的滚动文件,
// 不动同目录下其他程序的 .log)
void removeExpiredLocked()
{
    if(g_expireDays < 0 || g_base.isEmpty())
    {
        return;
    }
    const qint64 expireMs = g_expireDays * 24 * 3600 * 1000;
    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const QFileInfoList entries = QDir(g_dir).entryInfoList(QDir::Files);
    for(const QFileInfo &fi : entries)
    {
        if(!fi.fileName().startsWith(g_base + QLatin1Char('_')) ||
           fi.suffix().compare(QLatin1String("log"), Qt::CaseInsensitive) != 0)
        {
            continue;
        }
        const QDateTime born = fi.birthTime(); // 个别文件系统取不到创建时间,跳过以防误删
        if(born.isValid() && (now - born.toMSecsSinceEpoch()) > expireMs)
        {
            QFile::remove(fi.absoluteFilePath());
        }
    }
}

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    QMutexLocker lock(&g_mutex);
    if(g_file.isOpen())
    {
        // 跨天或超过单文件上限时滚动到新文件
        const QString date = QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd"));
        if(date != g_date || g_file.size() >= g_maxSize)
        {
            g_file.close();
            openFileLocked();
        }
        // 调用点信息:LOG_X 宏的消息里已自带;其余消息(Qt 内部等)从 context 补上,
        // 自家转发(context 指向 logger.cpp)不算调用方
        QString prefix;
        if(context.file && qstrcmp(context.file, g_selfFile) != 0)
        {
            prefix = QStringLiteral("[%1(%2)] ").arg(shortFile(context.file)).arg(context.line);
        }
        const QString line = QStringLiteral("[%1] [%2] %3%4\n")
            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy-MM-dd hh:mm:ss.zzz")),
                 levelName(type, context), prefix, message);
        g_file.write(line.toUtf8());
        g_file.flush();
    }
    if(g_previous)
    {
        g_previous(type, context, message);
    }
}

} // namespace

LogStream::LogStream(Log::Level level, const char *file, int line)
    : m_level(level),
      m_file(file ? shortFile(file) : QString()),
      m_line(line),
      m_stream(&m_buffer)
{
}

LogStream::~LogStream()
{
    m_stream.flush();
    QString text = m_buffer;
    if(!m_file.isEmpty())
    {
        text.prepend(QStringLiteral("[%1(%2)] ").arg(m_file).arg(m_line));
    }
    const QByteArray utf8 = text.toUtf8();
    switch(m_level)
    {
        case Log::Level::Trace:   qCDebug(lcTrace, "%s", utf8.constData()); break;
        case Log::Level::Debug:   qDebug("%s", utf8.constData()); break;
        case Log::Level::Info:    qInfo("%s", utf8.constData()); break;
        case Log::Level::Warning: qWarning("%s", utf8.constData()); break;
        case Log::Level::Error:   qCritical("%s", utf8.constData()); break;
        case Log::Level::Fatal:   qFatal("%s", utf8.constData()); break;
    }
}

namespace Log {

LogStream trace()   { return LogStream(Level::Trace); }
LogStream debug()   { return LogStream(Level::Debug); }
LogStream info()    { return LogStream(Level::Info); }
LogStream warning() { return LogStream(Level::Warning); }
LogStream error()   { return LogStream(Level::Error); }
LogStream fatal()   { return LogStream(Level::Fatal); }

bool setFile(const QString &path)
{
    QMutexLocker lock(&g_mutex);
    g_file.close();
    if(path.isEmpty())
    {
        return true;
    }
    const QFileInfo info(path);
    g_dir = info.absolutePath();
    g_base = info.completeBaseName();
    QDir().mkpath(g_dir);
    removeExpiredLocked();
    openFileLocked();
    if(!g_file.isOpen())
    {
        return false;
    }
    if(!g_previous)
    {
        g_previous = qInstallMessageHandler(messageHandler);
    }
    return true;
}

void setMaxSize(qint64 size)
{
    QMutexLocker lock(&g_mutex);
    if(size > 0)
    {
        g_maxSize = size;
    }
}

void setExpireDays(int days)
{
    QMutexLocker lock(&g_mutex);
    g_expireDays = days;
}

} // namespace Log
