#include <QtTest>

#include <QtNetwork>

#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>

#include "duration.h"
#include "httpfetch.h"
#include "logger.h"
#include "lrcparser.h"
#include "singleinstance.h"
#include "singleton.h"

// 本地迷你 HTTP 服务:应答固定状态码与响应体,记录最近一次请求头与 body。
// 全程 127.0.0.1,不碰外网;silent 模式收下请求但不应答,供超时用例使用
class MiniHttpServer : public QObject
{
public:
    explicit MiniHttpServer(QObject *parent = nullptr)
        : QObject(parent)
    {
        QObject::connect(&m_server, &QTcpServer::newConnection, this, [this]() {
            QTcpSocket *socket = m_server.nextPendingConnection();
            QObject::connect(socket, &QTcpSocket::readyRead, this, [this, socket]() {
                m_pending[socket] += socket->readAll();
                const int headerEnd = m_pending[socket].indexOf("\r\n\r\n");
                if(headerEnd < 0)
                {
                    return;
                }
                const QByteArray head = m_pending[socket].left(headerEnd);
                int contentLength = 0;
                const QList<QByteArray> lines = head.split('\n');
                for(const QByteArray &line : lines)
                {
                    // QByteArray::startsWith 没有 Qt::CaseInsensitive 重载,先转小写再比
                    if(line.trimmed().toLower().startsWith("content-length:"))
                    {
                        contentLength = line.mid(15).trimmed().toInt();
                    }
                }
                if(m_pending[socket].size() < headerEnd + 4 + contentLength)
                {
                    return;  // body 还没收完
                }
                m_lastRequestHead = head;
                m_lastBody = m_pending[socket].mid(headerEnd + 4, contentLength);
                m_pending.remove(socket);
                if(m_silent)
                {
                    return;  // 收下但不应答,耗到调用方超时
                }
                if(m_junk)
                {
                    socket->write("this is not http\r\n\r\n");
                    socket->flush();
                    socket->disconnectFromHost();
                    return;
                }
                const QByteArray payload = QByteArray("HTTP/1.0 ")
                    + QByteArray::number(m_status)
                    + " OK\r\nContent-Type: text/plain\r\nContent-Length: "
                    + QByteArray::number(m_fixedBody.size())
                    + "\r\nConnection: close\r\n\r\n" + m_fixedBody;
                socket->write(payload);
                socket->flush();
                socket->disconnectFromHost();
            });
            QObject::connect(socket, &QTcpSocket::disconnected,
                             socket, &QTcpSocket::deleteLater);
        });
        m_server.listen(QHostAddress::LocalHost);
    }

    QUrl url(const QString &path = QStringLiteral("/")) const
    {
        return QUrl(QStringLiteral("http://127.0.0.1:%1%2")
                        .arg(m_server.serverPort()).arg(path));
    }

    void respond(int status, const QByteArray &body)
    {
        m_status = status;
        m_fixedBody = body;
    }

    void setSilent(bool silent) { m_silent = silent; }

    // 应答非 HTTP 垃圾字节并断开:QNAM 立即报协议错,造快速可复现的 Error
    void setJunk(bool junk) { m_junk = junk; }
    QByteArray lastRequestHead() const { return m_lastRequestHead; }
    QByteArray lastBody() const { return m_lastBody; }

private:
    QTcpServer m_server;
    QHash<QTcpSocket *, QByteArray> m_pending;
    QByteArray m_lastRequestHead;
    QByteArray m_lastBody;
    QByteArray m_fixedBody = QByteArrayLiteral("hello");
    int m_status = 200;
    bool m_silent = false;
    bool m_junk = false;
};

class TstCore : public QObject
{
    Q_OBJECT
private slots:
    void singleton();
    void duration();
    void lrcParser();
    void logger();
    void loggerLevel();
    void httpFetchSync();
    void httpFetchSyncLocal();
    void httpFetchSyncTimeout();
    void httpFetchAsync();
    void httpFetchCancel();
    void singleInstance();
};

// CRTP 单例:同一类型两次取址相同,状态共享;不同类型实例互不相干
void TstCore::singleton()
{
    struct A : public Singleton<A> { int value = 0; };
    struct B : public Singleton<B> { int value = 0; };

    A::instance()->value = 42;
    QCOMPARE(A::instance(), A::instance());
    QCOMPARE(A::instance()->value, 42);
    QCOMPARE(B::instance()->value, 0);
    QVERIFY(static_cast<void *>(A::instance()) != static_cast<void *>(B::instance()));
}

void TstCore::duration()
{
    QCOMPARE(Duration::format(0), QStringLiteral("00:00"));
    QCOMPARE(Duration::format(59'000), QStringLiteral("00:59"));
    QCOMPARE(Duration::format(65'000), QStringLiteral("01:05"));
    QCOMPARE(Duration::format(3'600'000), QStringLiteral("1:00:00"));
    QCOMPARE(Duration::format(3'661'000), QStringLiteral("1:01:01"));
    // 负数按 0 处理
    QCOMPARE(Duration::format(-5'000), QStringLiteral("00:00"));

    QCOMPARE(Duration::parse(QStringLiteral("12")), qint64(12'000));
    QCOMPARE(Duration::parse(QStringLiteral("01:05")), qint64(65'000));
    QCOMPARE(Duration::parse(QStringLiteral("1:01:01")), qint64(3'661'000));
    // 非法输入返回 -1
    QCOMPARE(Duration::parse(QStringLiteral("abc")), qint64(-1));
    QCOMPARE(Duration::parse(QStringLiteral("-1:00")), qint64(-1));
    QCOMPARE(Duration::parse(QStringLiteral("1:2:3:4")), qint64(-1));

    // 往返一致
    QCOMPARE(Duration::parse(Duration::format(3'661'000)), qint64(3'661'000));
    QCOMPARE(Duration::parse(Duration::format(65'000)), qint64(65'000));
}

void TstCore::lrcParser()
{
    // 基本标签:两位/一位分钟、两三位小数、逗号小数
    const QString lrc = QStringLiteral(
        "[ti:晚风信笺]\n"
        "[ar:演示歌手]\n"
        "[al:控件库示例]\n"
        "[by:别人]\n"
        "[00:12.00]第一行\n"
        "[1:02,5]第二行\n"
        "[01:03.125][00:50]第三行双标签\n"
        "[00:20]\n"
        "没有标签的杂行\n"
        "[offset:+1000]\n");
    const LrcParser::Result result = LrcParser::parse(lrc);

    QCOMPARE(result.title, QStringLiteral("晚风信笺"));
    QCOMPARE(result.artist, QStringLiteral("演示歌手"));
    QCOMPARE(result.album, QStringLiteral("控件库示例"));

    // offset 全局生效:+1000 整体提前 1s;多标签拆行;乱序输入按时间排序
    QCOMPARE(result.lines.size(), 5);
    QCOMPARE(result.lines.at(0).ms, qint64(11'000));
    QCOMPARE(result.lines.at(0).text, QStringLiteral("第一行"));
    QCOMPARE(result.lines.at(1).ms, qint64(19'000));
    QCOMPARE(result.lines.at(1).text, QString());          // 空文本行保留
    QCOMPARE(result.lines.at(2).ms, qint64(49'000));
    QCOMPARE(result.lines.at(2).text, QStringLiteral("第三行双标签"));
    QCOMPARE(result.lines.at(3).ms, qint64(61'500));
    QCOMPARE(result.lines.at(3).text, QStringLiteral("第二行"));
    QCOMPARE(result.lines.at(4).ms, qint64(62'125));
    QCOMPARE(result.lines.at(4).text, QStringLiteral("第三行双标签"));

    // 无 offset:小数位数补齐(.5 = 500ms),秒一位数宽容
    const LrcParser::Result plain = LrcParser::parse(QStringLiteral("[00:01.5]a\n[00:02]b"));
    QCOMPARE(plain.lines.size(), 2);
    QCOMPARE(plain.lines.at(0).ms, qint64(1'500));
    QCOMPARE(plain.lines.at(1).ms, qint64(2'000));

    // 负 offset 延后;空文本与无文本标签行都可解析
    const LrcParser::Result delayed = LrcParser::parse(
        QStringLiteral("[offset:-500]\n[00:01.00]x"));
    QCOMPARE(delayed.lines.size(), 1);
    QCOMPARE(delayed.lines.at(0).ms, qint64(1'500));
}

void TstCore::logger()
{
    // 基础流式 + 宏调用点 + 节流:镜像文件为 <基准名>_<日期>_<序号>.log
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(Log::setFile(dir.filePath(QStringLiteral("app.log"))));

    LOG_TRACE << "trace text";
    Log::info() << "hello" << 42;
    LOG_WARNING << "warn text" << 7;
    for(int i = 0; i < 2; ++i)
    {
        LOG_INFO_ONCE("once text"); // ONCE 按调用点节流:同一展开点第二次循环被拦截
    }
    QVERIFY(Log::setFile(QString())); // 关闭文件后才能读取(Windows 文件占用)

    QDir outDir(dir.path());
    QVERIFY(outDir.entryList(QStringList(QStringLiteral("app_*.log")), QDir::Files).size() == 1);
    QFile file(outDir.filePath(outDir.entryList(QStringList(QStringLiteral("app_*.log")), QDir::Files).first()));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(content.contains(QStringLiteral("hello 42")));
    QVERIFY(content.contains(QStringLiteral("INFO")));
    QVERIFY(content.contains(QStringLiteral("trace text")));
    QVERIFY(content.contains(QStringLiteral("TRACE")));
    QVERIFY(content.contains(QStringLiteral("warn text 7")));
    QVERIFY(content.contains(QStringLiteral("WARN")));
    QVERIFY(content.contains(QStringLiteral("tst_core.cpp"))); // 宏捕获的调用点文件名
    QVERIFY(content.count(QStringLiteral("once text")) == 1);  // ONCE 只打一次

    // 轮转:maxSize=1 时每条消息都滚动到新序号文件
    QTemporaryDir dir2;
    QVERIFY(dir2.isValid());
    Log::setMaxSize(1);
    QVERIFY(Log::setFile(dir2.filePath(QStringLiteral("roll.log"))));
    Log::info() << "first";
    Log::info() << "second";
    QVERIFY(Log::setFile(QString()));
    QVERIFY(QDir(dir2.path()).entryList(QStringList(QStringLiteral("roll_*.log")), QDir::Files).size() == 2);

    // 过期清理:expire=0 时重开即清空旧滚动文件,只留新打开的一个
    Log::setExpireDays(0);
    QTest::qWait(5); // 保证 now - 创建时间 > 0
    QVERIFY(Log::setFile(dir2.filePath(QStringLiteral("roll.log"))));
    QVERIFY(Log::setFile(QString()));
    QVERIFY(QDir(dir2.path()).entryList(QStringList(QStringLiteral("roll_*.log")), QDir::Files).size() == 1);

    // 恢复默认,避免影响其他用例
    Log::setMaxSize(5 * 1024 * 1024);
    Log::setExpireDays(7);
}

void TstCore::loggerLevel()
{
    // Fatal 不测:qFatal 提交后程序中止,进不了断言
    QCOMPARE(Log::level(), Log::Level::Trace); // 默认全放行

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(Log::setFile(dir.filePath(QStringLiteral("lvl.log"))));
    Log::setLevel(Log::Level::Warning);

    // 三条应被过滤:流式 API 两条 + Qt 自身 qDebug 一条(处理器同样拦截)
    Log::info() << "dropped-info";
    LOG_DEBUG << "dropped-debug";
    qDebug("%s", "dropped-qdebug");
    // 两条应保留
    LOG_WARNING << "kept-warn";
    Log::error() << "kept-error";

    QVERIFY(Log::setFile(QString())); // 关闭文件后才能读取(Windows 文件占用)
    QDir outDir(dir.path());
    const auto files = outDir.entryList(QStringList(QStringLiteral("lvl_*.log")), QDir::Files);
    QCOMPARE(files.size(), 1);
    QFile file(outDir.filePath(files.first()));
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    file.close();

    QVERIFY(!content.contains(QStringLiteral("dropped")));
    QVERIFY(content.contains(QStringLiteral("kept-warn")));
    QVERIFY(content.contains(QStringLiteral("kept-error")));

    // 恢复全放行后 trace 重新可见
    Log::setLevel(Log::Level::Trace);
    QCOMPARE(Log::level(), Log::Level::Trace);
    QVERIFY(Log::setFile(dir.filePath(QStringLiteral("lvl.log"))));
    LOG_TRACE << "kept-trace";
    QVERIFY(Log::setFile(QString()));
    QFile file2(outDir.filePath(files.first()));
    QVERIFY(file2.open(QIODevice::ReadOnly | QIODevice::Text));
    QVERIFY(QString::fromUtf8(file2.readAll()).contains(QStringLiteral("kept-trace")));
    file2.close();
}

void TstCore::httpFetchSync()
{
    // 机制:UA 默认空(线上用内置 "canfan-common")、证书校验默认开启、初始空闲
    HttpFetch fetcher;
    QCOMPARE(fetcher.userAgent(), QString());
    QVERIFY(!fetcher.insecureMode());
    QVERIFY(!fetcher.isBusy());

    // 垃圾应答 -> 立即 Error,与超时/空 body 可区分
    // (不用 127.0.0.1:1 造拒连:本机低端口拒连要数秒才到,会先撞上超时)
    MiniHttpServer server;
    server.setJunk(true);
    const auto junk = HttpFetch::syncGet(server.url(), 3000);
    QVERIFY(!junk.ok());
    QCOMPARE(junk.status, HttpFetch::Result::Status::Error);
    QVERIFY(!junk.error.isEmpty());
    QVERIFY(junk.body.isEmpty());
}

void TstCore::httpFetchSyncLocal()
{
    MiniHttpServer server;
    server.respond(200, QByteArrayLiteral("hello"));

    const auto ok = HttpFetch::syncGet(server.url(), 3000);
    QVERIFY(ok.ok());
    QCOMPARE(ok.status, HttpFetch::Result::Status::Success);
    QCOMPARE(ok.body, QByteArrayLiteral("hello"));
    QCOMPARE(ok.httpStatus, 200);
    QVERIFY(server.lastRequestHead().contains("canfan-common")); // 内置 UA 已生效

    // HTTP 404 -> Error,错误串带状态码可读
    server.respond(404, QByteArrayLiteral("nope"));
    const auto missing = HttpFetch::syncGet(server.url(QStringLiteral("/missing")), 3000);
    QVERIFY(!missing.ok());
    QCOMPARE(missing.httpStatus, 404);
    QVERIFY(missing.error.contains(QStringLiteral("404")));

    // POST:body 与默认表单类型都已送达
    server.respond(200, QByteArrayLiteral("echo-back"));
    const auto posted =
        HttpFetch::syncPost(server.url(QStringLiteral("/submit")),
                            QByteArrayLiteral("a=1&b=2"), 3000);
    QVERIFY(posted.ok());
    QCOMPARE(server.lastBody(), QByteArrayLiteral("a=1&b=2"));

    // 同步下载:写入文件并覆盖已有内容
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("cover.bin"));
    {
        QFile pre(path);
        QVERIFY(pre.open(QIODevice::WriteOnly));
        pre.write("old");
    }
    server.respond(200, QByteArrayLiteral("new-bytes"));
    const auto downloaded =
        HttpFetch::syncDownload(server.url(QStringLiteral("/cover")), path, 3000);
    QVERIFY(downloaded.ok());
    QFile check(path);
    QVERIFY(check.open(QIODevice::ReadOnly));
    QCOMPARE(check.readAll(), QByteArrayLiteral("new-bytes"));
}

void TstCore::httpFetchSyncTimeout()
{
    MiniHttpServer server;   // silent:不应答
    server.setSilent(true);

    QElapsedTimer clock;
    clock.start();
    const auto timeout = HttpFetch::syncGet(server.url(), 300);
    QVERIFY(!timeout.ok());
    QCOMPARE(timeout.status, HttpFetch::Result::Status::Timeout);
    QVERIFY(!timeout.error.isEmpty());
    QVERIFY(clock.elapsed() < 5000);  // 到点即返,不能真耗到网络层超时
}

void TstCore::httpFetchAsync()
{
    HttpFetch fetcher;
    QSignalSpy failedSpy(&fetcher, &HttpFetch::failed);
    QSignalSpy finishedSpy(&fetcher, &HttpFetch::finished);
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("partial.bin"));

    // 下载到拒连地址:failed、无 finished、不留半截文件
    fetcher.download(QUrl(QStringLiteral("http://127.0.0.1:1/x")), path);
    QVERIFY(fetcher.isBusy());
    QTRY_COMPARE(failedSpy.count(), 1);
    QCOMPARE(finishedSpy.count(), 0);
    QVERIFY(!fetcher.isBusy());
    QVERIFY(!QFile::exists(path));

    // 忙时新请求被忽略
    QVERIFY(failedSpy.count() >= 1);
    const int before = failedSpy.count();
    fetcher.get(QUrl(QStringLiteral("http://127.0.0.1:1/x")));
    fetcher.get(QUrl(QStringLiteral("http://127.0.0.1:1/y")));
    QTRY_COMPARE(failedSpy.count(), before + 1);
}

void TstCore::httpFetchCancel()
{
    MiniHttpServer server;
    server.setSilent(true);
    HttpFetch fetcher;
    QSignalSpy failedSpy(&fetcher, &HttpFetch::failed);

    fetcher.get(server.url());
    QVERIFY(fetcher.isBusy());
    fetcher.cancel();
    QTRY_COMPARE(failedSpy.count(), 1);
    QVERIFY(!fetcher.isBusy());
}

void TstCore::singleInstance()
{
    const QString key = QStringLiteral("tst-single-") + QString::number(QCoreApplication::applicationPid());

    SingleInstance primary(key);
    QVERIFY(primary.isPrimary());
    QVERIFY(!primary.sendMessage(QStringLiteral("x"))); // 主实例自己不发消息

    // 副实例用子进程模拟(真实场景:副实例一定是另一个进程,
    // 同线程内的两个 SingleInstance 受 QLocalSocket 事件循环限制无法收发)。
    // 用 QTRY 而不是 waitForFinished:主实例要靠这段循环泵事件来收消息、回确认,
    // 与真实应用(事件循环常驻)一致
    QSignalSpy spy(&primary, &SingleInstance::messageReceived);
    QProcess secondary;
    secondary.setProgram(QCoreApplication::applicationFilePath());
    secondary.setArguments({key, QStringLiteral("--secondary")});
    secondary.start();
    QTRY_VERIFY_WITH_TIMEOUT(secondary.state() == QProcess::NotRunning, 15000);
    QCOMPARE(secondary.exitCode(), 0);

    QTRY_COMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).at(0).toString(), QStringLiteral("activate"));
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    // 本机若配了系统代理会劫持 127.0.0.1 的 HTTP 测试请求,测试进程一律直连
    QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    const QStringList args = app.arguments();
    if(args.size() >= 3 && args.at(2) == QStringLiteral("--secondary"))
    {
        SingleInstance guard(args.at(1));
        if(guard.isPrimary())
        {
            return 2; // 不应成为主实例
        }
        return guard.sendMessage(QStringLiteral("activate")) ? 0 : 1;
    }

    TstCore tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "tst_core.moc"
