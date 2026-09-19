#pragma once

#include <QWidget>

class QLabel;
class QPropertyAnimation;
class QTimer;

// 桌面通知:主屏右下角弹出的悬浮通知窗(切歌提示、下载完成等),
// duration 毫秒后自动淡出销毁;多条通知自下而上堆叠。
// 便捷用法:NotifyWindow::showMessage("标题", "内容");
// 点击通知发出 clicked() 并关闭
class NotifyWindow : public QWidget
{
    Q_OBJECT
public:
    // 弹出一条通知并返回实例(自动销毁,无需手动管理)
    static NotifyWindow *showMessage(const QString &title, const QString &text,
                                     int duration = 3500);

    ~NotifyWindow() override;

signals:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    explicit NotifyWindow(const QString &title, const QString &text);

    void dismiss();          // 淡出并销毁
    void restack(int index); // 挪到堆叠层的第 index 个位置

    QLabel *m_title = nullptr;
    QLabel *m_text = nullptr;
    QTimer *m_timer = nullptr;
    QPropertyAnimation *m_fade = nullptr;
};
