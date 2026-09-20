#include "toastlabel.h"

#include <QGraphicsOpacityEffect>
#include <QGuiApplication>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>

#include "theme.h"

ToastLabel::ToastLabel(const QString &text, QWidget *parent)
    : QLabel(text, parent)
{
    setAlignment(Qt::AlignCenter);
    setMargin(12);
    setAttribute(Qt::WA_TransparentForMouseEvents); // 不遮挡下层控件的点击
    applyThemeStyle();
    connect(Theme::instance(), &Theme::themeChanged, this, &ToastLabel::applyThemeStyle);

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

void ToastLabel::applyThemeStyle()
{
    // 反色表面(亮暗模式都取深底浅字,与 Toast 的提示定位一致)
    Theme *theme = Theme::instance();
    setStyleSheet(QStringLiteral("ToastLabel{background:%1;color:%2;border-radius:%3px;}")
                      .arg(theme->color(Theme::Role::InverseSurface).name())
                      .arg(theme->color(Theme::Role::InverseText).name())
                      .arg(theme->radius(Tokens::Radius::MD)));
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
