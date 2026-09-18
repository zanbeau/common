#include <QtTest>

#include <QFile>
#include <QProcess>
#include <QTemporaryDir>

#include "duration.h"
#include "logger.h"
#include "singleinstance.h"

class TstCore : public QObject
{
    Q_OBJECT
private slots:
    void duration();
    void logger();
    void singleInstance();
};

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
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("log.txt"));
    QVERIFY(Log::setFile(path));

    Log::info() << "hello" << 42;
    Log::warning() << "warn text";
    QVERIFY(Log::setFile(QString())); // 关闭文件后才能读取(Windows 文件占用)

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(file.readAll());
    QVERIFY(content.contains(QStringLiteral("hello 42")));
    QVERIFY(content.contains(QStringLiteral("INFO")));
    QVERIFY(content.contains(QStringLiteral("warn text")));
    QVERIFY(content.contains(QStringLiteral("WARN")));
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
