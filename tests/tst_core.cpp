#include <QtTest>

#include <QDir>
#include <QFile>
#include <QProcess>
#include <QTemporaryDir>

#include "duration.h"
#include "logger.h"
#include "singleinstance.h"
#include "singleton.h"

class TstCore : public QObject
{
    Q_OBJECT
private slots:
    void singleton();
    void duration();
    void logger();
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
