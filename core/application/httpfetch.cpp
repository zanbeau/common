#include "httpfetch.h"

#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSaveFile>
#include <QTimer>

namespace {

constexpr const char kDefaultUserAgent[] = "canfan-common";
constexpr const char kFormContentType[] = "application/x-www-form-urlencoded";

// 重定向统一走 NoLessSafeRedirectPolicy:Qt5 默认不跟随、Qt6 默认值不同,
// 显式设置两边一致。Qt 5.15 没有 setRedirectPolicy(),属性形式两个版本都可用
void applyRedirectPolicy(QNetworkRequest &request)
{
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                         static_cast<int>(QNetworkRequest::NoLessSafeRedirectPolicy));
}

// insecure 仅对象接口支持(显式降级须 intentional);同步便捷函数恒走证书校验
QNetworkRequest prepareRequest(const QUrl &url, const QString &userAgent, bool insecure)
{
    QNetworkRequest request(url);
    applyRedirectPolicy(request);
    request.setRawHeader("User-Agent",
                         userAgent.isEmpty() ? QByteArray(kDefaultUserAgent)
                                             : userAgent.toUtf8());
#ifndef QT_NO_SSL
    if(insecure)
    {
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(config);
    }
#else
    Q_UNUSED(insecure);
#endif
    return request;
}

// 请求收尾判定:网络错误 / HTTP >= 400 视为失败,否则返回响应体
HttpFetch::Result evaluateReply(QNetworkReply *reply)
{
    HttpFetch::Result result;
    result.httpStatus =
        reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if(reply->error() != QNetworkReply::NoError)
    {
        // 带上状态码:4xx/5xx 在 Qt 里走协议错误,errorString 只有 reason phrase
        // (如 "server replied: OK"),不拼状态码就分不清 404 和 500
        result.error = (result.httpStatus > 0)
            ? QStringLiteral("HTTP %1: %2").arg(result.httpStatus).arg(reply->errorString())
            : reply->errorString();
        return result;
    }
    if(result.httpStatus >= 400)
    {
        result.error = QStringLiteral("HTTP %1").arg(result.httpStatus);
        return result;
    }
    result.status = HttpFetch::Result::Status::Success;
    result.body = reply->readAll();
    return result;
}

// 同步等待的公共实现:栈上 QNAM + 事件循环 + 单次定时器。
// 超时会真的 abort 掉请求,并以 Timeout 状态返回(与空 body 可区分)
HttpFetch::Result syncWait(QNetworkReply *reply, int timeoutMs)
{
    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, reply, &QNetworkReply::abort);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start(timeoutMs);
    loop.exec();
    reply->deleteLater();
    if(timer.isActive())
    {
        timer.stop();  // 请求在超时前完成
        return evaluateReply(reply);
    }
    // 定时器先到点:reply 已被 abort,finished 稍后自会到,这里直接判超时
    HttpFetch::Result result;
    result.status = HttpFetch::Result::Status::Timeout;
    result.error = QStringLiteral("request timed out after %1 ms").arg(timeoutMs);
    return result;
}

} // namespace

HttpFetch::HttpFetch(QObject *parent)
    : QObject(parent)
{
    connect(&m_manager, &QNetworkAccessManager::finished,
            this, &HttpFetch::onFinished);
}

HttpFetch::~HttpFetch()
{
    // 防御式收尾:在途请求直接中止,半截下载丢弃
    if(m_reply)
    {
        m_reply->disconnect(this);
        m_reply->abort();
        m_reply->deleteLater();
    }
    if(m_saveFile)
    {
        m_saveFile->cancelWriting();
    }
}

void HttpFetch::setUserAgent(const QString &userAgent)
{
    m_userAgent = userAgent;
}

QString HttpFetch::userAgent() const
{
    return m_userAgent;
}

void HttpFetch::setInsecureMode(bool insecure)
{
    m_insecure = insecure;
}

bool HttpFetch::insecureMode() const
{
    return m_insecure;
}

bool HttpFetch::isBusy() const
{
    return m_reply != nullptr;
}

void HttpFetch::cancel()
{
    if(m_reply)
    {
        m_reply->abort();  // abort 触发 finished -> onFinished 上报失败
    }
}

void HttpFetch::get(const QUrl &url)
{
    startRequest(url, false, QByteArray(), QByteArray());
}

void HttpFetch::post(const QUrl &url, const QByteArray &body, const QByteArray &contentType)
{
    startRequest(url, true, body, contentType);
}

void HttpFetch::download(const QUrl &url, const QString &filePath)
{
    if(m_reply)
    {
        qWarning("HttpFetch: busy, download ignored");
        return;
    }
    m_downloadPath = filePath;
    m_saveFile = new QSaveFile(filePath, this);
    if(!m_saveFile->open(QIODevice::WriteOnly))
    {
        const QString error = m_saveFile->errorString();
        delete m_saveFile;
        m_saveFile = nullptr;
        emit failed(error);
        return;
    }
    startRequest(url, false, QByteArray(), QByteArray());
}

HttpFetch::Result HttpFetch::syncGet(const QUrl &url, int timeoutMs)
{
    QNetworkAccessManager manager;
    QNetworkReply *reply = manager.get(prepareRequest(url, QString(), false));
    return syncWait(reply, timeoutMs);
}

HttpFetch::Result HttpFetch::syncPost(const QUrl &url, const QByteArray &body, int timeoutMs)
{
    QNetworkAccessManager manager;
    QNetworkRequest request = prepareRequest(url, QString(), false);
    if(!body.isEmpty())
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArray(kFormContentType));
    }
    QNetworkReply *reply = manager.post(request, body);
    return syncWait(reply, timeoutMs);
}

HttpFetch::Result HttpFetch::syncDownload(const QUrl &url, const QString &filePath,
                                          int timeoutMs)
{
    const Result fetched = syncGet(url, timeoutMs);
    if(!fetched.ok())
    {
        return fetched;
    }
    QSaveFile file(filePath);
    if(file.open(QIODevice::WriteOnly) && file.write(fetched.body) != -1 && file.commit())
    {
        return fetched;
    }
    HttpFetch::Result failed;
    failed.error = file.errorString();
    failed.httpStatus = fetched.httpStatus;
    return failed;
}

void HttpFetch::startRequest(const QUrl &url, bool isPost, const QByteArray &body,
                             const QByteArray &contentType)
{
    if(m_reply)
    {
        qWarning("HttpFetch: busy, request ignored");
        return;
    }
    QNetworkRequest request = prepareRequest(url, m_userAgent, m_insecure);
    if(!contentType.isEmpty())
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }
    else if(isPost && !body.isEmpty())
    {
        // 未指定类型时给 POST 补常用表单编码,与 syncPost 一致
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArray(kFormContentType));
    }
    m_reply = isPost ? m_manager.post(request, body) : m_manager.get(request);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &HttpFetch::progress);
    connect(m_reply, &QNetworkReply::readyRead, this, [this]() {
        if(m_saveFile)
        {
            m_saveFile->write(m_reply->readAll());  // download 边收边落盘
        }
    });
}

void HttpFetch::onFinished()
{
    if(!m_reply)
    {
        return;
    }
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    disconnect(reply, nullptr, this, nullptr);

    if(m_saveFile)
    {
        QSaveFile *file = m_saveFile;
        m_saveFile = nullptr;
        const QString path = m_downloadPath;
        m_downloadPath.clear();
        const Result result = evaluateReply(reply);
        reply->deleteLater();
        if(result.ok() && file->commit())
        {
            emit downloaded(path);
        }
        else
        {
            file->cancelWriting();  // 丢弃半截文件,不落残留
            emit failed(result.ok() ? file->errorString() : result.error);
        }
        file->deleteLater();
        return;
    }

    const Result result = evaluateReply(reply);
    reply->deleteLater();
    if(result.ok())
    {
        emit finished(result.body);
    }
    else
    {
        emit failed(result.error);
    }
}
