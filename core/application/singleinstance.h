#pragma once

#include <QHash>
#include <QLocalServer>
#include <QLocalSocket>
#include <QObject>

// 单实例守护:防止应用双开
// 第一个构造的实例成为主实例(isPrimary() 为 true)并监听消息;
// 后续实例通过 sendMessage() 把消息转发给主实例后自行退出。
// 组合式设计,不继承 QApplication —— 任意应用类都可用,也不给 core 引入 UI 依赖。
class SingleInstance : public QObject
{
    Q_OBJECT
public:
    explicit SingleInstance(const QString &key, QObject *parent = nullptr);

    bool isPrimary() const;

    // 副实例向主实例发送消息(换行分帧),收到主实例的确认后返回 true;
    // 主实例收到消息会发出 messageReceived 信号(需要主实例事件循环保持响应)
    bool sendMessage(const QString &message, int timeout = 1000);

signals:
    void messageReceived(const QString &message);

private:
    void onNewConnection();
    void onReadyRead();
    void processMessage(QLocalSocket *socket, bool closing);

    const QString m_key;
    bool m_primary = false;
    QLocalServer m_server;
    QHash<QLocalSocket *, QByteArray> m_buffers;
};
