#include <QtTest>

#include <QFile>
#include <QProcess>
#include <QTemporaryDir>

#include "logger.h"
#include "singleinstance.h"

class TstCore : public QObject
{
    Q_OBJECT
private slots:
    void logger();
    void singleInstance();
};

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
