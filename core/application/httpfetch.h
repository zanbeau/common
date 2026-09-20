#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>

class QNetworkReply;
class QSaveFile;

// HTTP 拉取:异步 get/post/download + 事件循环式同步等待,面向
// 封面/歌词/在线检索这类"取回内容"的场景(借自 TTKAbstractNetwork/TTKEventLoop
// 的改良版):
//   - 同步等待超时会真的 abort() 掉请求(TTK 只退出循环,请求仍在后台跑)
//   - 同步结果用 Result 区分 成功/超时/错误(TTK 里超时与空 body 不可分)
//   - 默认保持 SSL 证书校验(TTK 默认关掉;内网自签用 setInsecureMode 显式降级)
//   - 重定向统一 NoLessSafeRedirectPolicy(Qt5 默认不跟随、Qt6 默认值不同),
//     body 永远是最终地址的内容
// 每个对象一个 QNetworkAccessManager,按用途各建一个(封面/歌词/检索)即可;
// 同一对象同时只跑一个请求,忙时新请求被忽略。异步请求没有内建超时,
// 需要时用 cancel() 主动中止
class HttpFetch : public QObject
{
    Q_OBJECT
public:
    // 同步便捷函数的返回值;异步路径用信号,不走这里
    struct Result
    {
        enum class Status
        {
            Success,
            Timeout,
            Error
        };

        Status status = Status::Error;
        QByteArray body;      // Success:最终响应体(重定向已跟随)
        QString error;        // 非 Success:可读错误(网络错误串 / "HTTP 404" / 超时说明)
        int httpStatus = 0;   // 拿得到时的 HTTP 状态码;Success 时一律 2xx

        bool ok() const { return status == Status::Success; }
    };

    explicit HttpFetch(QObject *parent = nullptr);
    ~HttpFetch() override;

    void setUserAgent(const QString &userAgent);  // 默认 "canfan-common"
    QString userAgent() const;

    // true = 关闭 SSL 证书校验,仅限内网自签场景显式开启;同步便捷函数不受影响
    void setInsecureMode(bool insecure);
    bool insecureMode() const;

    bool isBusy() const;

    // 中止在途请求,经 failed() 上报
    void cancel();

    // 异步请求;结果经 finished/downloaded/failed 上报,download 另有 progress
    void get(const QUrl &url);
    // contentType 为空且 body 非空时按表单编码补默认,自定义类型直接传
    void post(const QUrl &url, const QByteArray &body = QByteArray(),
              const QByteArray &contentType = QByteArray());
    // 下载到文件:QSaveFile 原子提交,覆盖同名文件,失败不残留半截文件
    void download(const QUrl &url, const QString &filePath);

    // —— 同步便捷:内部事件循环 + 单次定时器,到点 abort,状态可区分 ——
    static Result syncGet(const QUrl &url, int timeoutMs = 10000);
    static Result syncPost(const QUrl &url, const QByteArray &body = QByteArray(),
                           int timeoutMs = 10000);
    static Result syncDownload(const QUrl &url, const QString &filePath,
                               int timeoutMs = 10000);

signals:
    void finished(const QByteArray &body);      // get/post 成功
    void downloaded(const QString &filePath);   // download 成功
    void failed(const QString &error);          // 失败/取消
    void progress(qint64 received, qint64 total);  // total < 0 表示无 Content-Length

private:
    void startRequest(const QUrl &url, bool isPost, const QByteArray &body,
                      const QByteArray &contentType);
    void onFinished();

    QNetworkReply *m_reply = nullptr;
    QSaveFile *m_saveFile = nullptr;   // 仅 download 非空
    QString m_downloadPath;
    QNetworkAccessManager m_manager;
    QString m_userAgent;
    bool m_insecure = false;
};
