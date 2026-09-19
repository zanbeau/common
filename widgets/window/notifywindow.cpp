#include "notifywindow.h"

#include <QGuiApplication>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

#include "theme.h"

namespace {

// 活跃通知登记表:堆叠定位用;窗口销毁时移除并把下方的顶上来
QList<NotifyWindow *> g_active;

constexpr int kNotifyWidth = 320;
constexpr int kNotifyHeight = 84;
constexpr int kNotifyGap = 8;       // 通知之间的垂直间距
constexpr int kScreenMargin = 12;   // 距屏幕右下角的边距

} // namespace

NotifyWindow::NotifyWindow(const QString &title, const QString &text)
    : QWidget(nullptr, Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(kNotifyWidth, kNotifyHeight);

    m_title = new QLabel(title, this);
    m_title->setCursor(Qt::PointingHandCursor);
    QFont titleFont = m_title->font();
    titleFont.setPixelSize(Theme::instance()->fontPx(Tokens::FontSize::BodyStrong));
    titleFont.setBold(true);
    m_title->setFont(titleFont);

    m_text = new QLabel(text, this);
    m_text->setCursor(Qt::PointingHandCursor);
    QFont textFont = m_text->font();
    textFont.setPixelSize(Theme::instance()->fontPx(Tokens::FontSize::Body));
    m_text->setFont(textFont);

    // 标签不吃鼠标,点击落到窗口本身
    m_title->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_text->setAttribute(Qt::WA_TransparentForMouseEvents);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(16, 14, 16, 14);
    layout->setSpacing(4);
    layout->addWidget(m_title);
    layout->addWidget(m_text);

    connect(Theme::instance(), &Theme::modeChanged, this, [this]() { update(); });

    // 入场淡入;到点后淡出销毁
    m_fade = new QPropertyAnimation(this, "windowOpacity", this);
    m_fade->setDuration(Theme::instance()->duration(Tokens::Duration::Fast));
    m_fade->setStartValue(0.0);
    m_fade->setEndValue(1.0);
    m_fade->start();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &NotifyWindow::dismiss);
    m_timer->start();
}

NotifyWindow::~NotifyWindow()
{
    const int index = g_active.indexOf(this);
    if(index >= 0)
    {
        g_active.removeAt(index);
        // 下方的通知整体上移一格
        for(int i = index; i < g_active.size(); ++i)
        {
            g_active.at(i)->restack(i);
        }
    }
}

NotifyWindow *NotifyWindow::showMessage(const QString &title, const QString &text,
                                        int duration)
{
    NotifyWindow *notify = new NotifyWindow(title, text);
    g_active.append(notify);
    notify->restack(g_active.size() - 1);
    notify->setWindowOpacity(0.0);
    notify->show();
    notify->m_timer->setInterval(qMax(duration, 500));
    return notify;
}

void NotifyWindow::restack(int index)
{
    const QRect available =
        QGuiApplication::primaryScreen()->availableGeometry();
    const int bottom = available.bottom() - kScreenMargin
                     - index * (kNotifyHeight + kNotifyGap);
    move(available.right() - kScreenMargin - kNotifyWidth, bottom - kNotifyHeight);
}

void NotifyWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && rect().contains(event->pos()))
    {
        emit clicked();
        dismiss();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void NotifyWindow::dismiss()
{
    m_timer->stop();
    m_fade->stop();
    m_fade->setStartValue(windowOpacity());
    m_fade->setEndValue(0.0);
    m_fade->setDuration(Theme::instance()->duration(Tokens::Duration::Normal));
    connect(m_fade, &QPropertyAnimation::finished, this, &NotifyWindow::close);
    m_fade->start();
}

void NotifyWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 圆角卡片:表面色 + 描边,左侧一条主题色竖线强化"通知"识别
    QPainterPath card;
    card.addRoundedRect(rect().adjusted(0, 0, -1, -1),
                        theme->radius(Tokens::Radius::LG),
                        theme->radius(Tokens::Radius::LG));
    painter.fillPath(card, theme->color(Theme::Role::Surface));
    QPen pen(theme->color(Theme::Role::Border));
    painter.setPen(pen);
    painter.drawPath(card);

    QPainterPath accent;
    accent.addRoundedRect(QRect(0, 10, 3, height() - 20), 1, 1);
    painter.fillPath(accent, theme->color(Theme::Role::Primary));

    m_title->setStyleSheet(QStringLiteral("color:%1;background:transparent;")
                               .arg(theme->color(Theme::Role::Text).name()));
    m_text->setStyleSheet(QStringLiteral("color:%1;background:transparent;")
                              .arg(theme->color(Theme::Role::TextSecondary).name()));
}
