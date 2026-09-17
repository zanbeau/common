#pragma once

#include <QLabel>

class QGraphicsOpacityEffect;
class QPropertyAnimation;
class QTimer;

// 气泡提示:popup() 后居中于父窗口显示,停留片刻后淡出
// 便捷用法:ToastLabel::showText("提示内容", this);
class ToastLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ToastLabel(const QString &text = {}, QWidget *parent = nullptr);

    // duration: 停留时间(毫秒);fadeDuration: 淡出动画时长(毫秒)
    void popup(int duration = 1600, int fadeDuration = 350);

    // 在 parent 中央弹出一条提示,结束后自动销毁
    static void showText(const QString &text, QWidget *parent,
                         int duration = 1600, int fadeDuration = 350);

private:
    void startFade();

    QGraphicsOpacityEffect *m_effect = nullptr;
    QPropertyAnimation *m_fade = nullptr;
    QTimer *m_timer = nullptr;
};
