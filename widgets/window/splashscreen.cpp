#include "splashscreen.h"

#include <QGuiApplication>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QShowEvent>

#include "theme.h"

SplashScreen::SplashScreen(const QPixmap &pixmap, QWidget *parent)
    : QWidget(parent, Qt::SplashScreen | Qt::FramelessWindowHint
              | Qt::WindowStaysOnTopHint)
    , m_pixmap(pixmap)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(420, 280);

    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

void SplashScreen::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    update();
}

QPixmap SplashScreen::pixmap() const
{
    return m_pixmap;
}

void SplashScreen::showMessage(const QString &message)
{
    m_message = message;
    update();
}

QString SplashScreen::message() const
{
    return m_message;
}

void SplashScreen::showEvent(QShowEvent *event)
{
    // 居中于主屏
    const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
    move(available.center() - QPoint(width() / 2, height() / 2));
    QWidget::showEvent(event);
}

void SplashScreen::finish(QWidget *mainWindow)
{
    if(mainWindow)
    {
        mainWindow->show();
        mainWindow->activateWindow();
        mainWindow->raise();
    }

    if(!m_fade)
    {
        m_fade = new QPropertyAnimation(this, "windowOpacity", this);
        m_fade->setDuration(Theme::instance()->duration(Tokens::Duration::Slow));
        m_fade->setStartValue(1.0);
        m_fade->setEndValue(0.0);
        connect(m_fade, &QPropertyAnimation::finished, this, &SplashScreen::close);
    }
    if(m_fade->state() != QAbstractAnimation::Running)
    {
        m_fade->start();
    }
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 圆角卡片
    QPainterPath card;
    card.addRoundedRect(rect().adjusted(0, 0, -1, -1),
                        theme->radius(Tokens::Radius::LG),
                        theme->radius(Tokens::Radius::LG));
    painter.fillPath(card, theme->color(Theme::Role::Surface));
    QPen pen(theme->color(Theme::Role::Border));
    painter.setPen(pen);
    painter.drawPath(card);

    // logo 居中(等比缩放,最大占卡片 60%)
    if(!m_pixmap.isNull())
    {
        const int extent = qMin(width(), height()) * 3 / 5;
        const QPixmap scaled = m_pixmap.scaled(extent, extent,
                                                Qt::KeepAspectRatio,
                                                Qt::SmoothTransformation);
        painter.drawPixmap((width() - scaled.width()) / 2,
                           (height() - scaled.height()) / 2 - 16, scaled);
    }

    // 底部状态文字
    if(!m_message.isEmpty())
    {
        QFont msgFont = font();
        msgFont.setPixelSize(theme->fontPx(Tokens::FontSize::Caption));
        painter.setFont(msgFont);
        painter.setPen(theme->color(Theme::Role::TextSecondary));
        painter.drawText(QRect(0, height() - 44, width(), 24),
                         Qt::AlignHCenter | Qt::AlignVCenter, m_message);
    }
}
