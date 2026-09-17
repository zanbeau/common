#include "singleinstance.h"

SingleInstance::SingleInstance(const QString &key, QObject *parent)
    : QObject(parent),
      m_key(key)
{
    // 先探测是否已有主实例在运行
    QLocalSocket probe;
    probe.connectToServer(m_key);
    if(probe.waitForConnected(300))
    {
        m_primary = false;
        return;
    }

    // 没有主实例,自己成为主实例;顺带清掉上次崩溃残留的服务器名
    QLocalServer::removeServer(m_key);
    connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
    m_primary = m_server.listen(m_key);
}

bool SingleInstance::isPrimary() const
{
    return m_primary;
}

bool SingleInstance::sendMessage(const QString &message, int timeout)
{
    if(m_primary)
    {
        return false;
    }

    QLocalSocket socket;
    socket.connectToServer(m_key);
    if(!socket.waitForConnected(timeout))
    {
        qWarning("SingleInstance: connect failed: %s", qUtf8Printable(socket.errorString()));
        return false;
    }

    // flush() 把数据同步交给系统管道缓冲;主实例处理完消息会回发确认,
    // 等到确认再返回,保证副实例退出前消息已被主实例处理
    socket.write(message.toUtf8() + '\n');
    socket.flush();
    if(!socket.waitForReadyRead(timeout))
    {
        qWarning("SingleInstance: no ack from primary instance");
        return false;
    }
    return true;
}

void SingleInstance::onNewConnection()
{
    while(QLocalSocket *client = m_server.nextPendingConnection())
    {
        qWarning("SingleInstance: client connected");
        connect(client, &QLocalSocket::readyRead, this, &SingleInstance::onReadyRead);
        connect(client, &QLocalSocket::disconnected, this, [this, client]() {
            processMessage(client, true);
            m_buffers.remove(client);
            client->deleteLater();
        });
    }
}

void SingleInstance::onReadyRead()
{
    if(auto *socket = qobject_cast<QLocalSocket *>(sender()))
    {
        m_buffers[socket] += socket->readAll();
        processMessage(socket, false);
    }
}

void SingleInstance::processMessage(QLocalSocket *socket, bool closing)
{
    QByteArray &buffer = m_buffers[socket];
    int start = 0;
    int index;
    while((index = buffer.indexOf('\n', start)) >= 0)
    {
        const QString message = QString::fromUtf8(buffer.mid(start, index - start));
        qWarning("SingleInstance: message received '%s'", qUtf8Printable(message));
        emit messageReceived(message);
        socket->write("ack\n"); // 告知副实例:消息已处理
        socket->flush();
        start = index + 1;
    }
    buffer.remove(0, start);

    if(closing && !buffer.isEmpty())
    {
        // 没有结尾换行的最后一段
        const QString message = QString::fromUtf8(buffer);
        qWarning("SingleInstance: message received '%s'", qUtf8Printable(message));
        emit messageReceived(message);
        socket->write("ack\n");
        socket->flush();
        buffer.clear();
    }
}
