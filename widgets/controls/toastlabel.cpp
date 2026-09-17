#include "toastlabel.h"

#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

ToastLabel::ToastLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setAlignment(Qt::AlignCenter);
    setMargin(12);
    setAttribute(Qt::WA_TransparentForMouseEvents); // 不遮挡下层控件的点击
    setStyleSheet(QStringLiteral("ToastLabel{background:rgba(40,40,40,215);"
                                 "color:#ffffff;border-radius:8px;}"));

    m_effect = new QGraphicsOpacityEffect(this);
    m_effect->setOpacity(1.0);
    setGraphicsEffect(m_effect);

    m_fade = new QPropertyAnimation(m_effect, "opacity", this);
    m_fade->setStartValue(1.0);
    m_fade->setEndValue(0.0);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ToastLabel::startFade);
    connect(m_fade, &QPropertyAnimation::finished, this, &QWidget::hide);
}

void ToastLabel::popup(int duration, int fadeDuration)
{
    if(QWidget *parent = parentWidget())
    {
        adjustSize();
        move((parent->width() - width()) / 2, (parent->height() - height()) / 2);
    }
    else
    {
        setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        adjustSize();
        const QRect screen = QGuiApplication::primaryScreen()->availableGeometry();
        move(screen.center() - rect().center());
    }

    m_fade->stop();
    m_fade->setDuration(fadeDuration);
    m_effect->setOpacity(1.0);
    show();
    raise();
    m_timer->start(duration);
}

void ToastLabel::startFade()
{
    m_fade->start();
}

void ToastLabel::showText(const QString &text, QWidget *parent,
                          int duration, int fadeDuration)
{
    auto *toast = new ToastLabel(text, parent);
    connect(toast->m_fade, &QPropertyAnimation::finished, toast, &QObject::deleteLater);
    toast->popup(duration, fadeDuration);
}
