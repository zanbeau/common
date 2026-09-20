#include "pushbutton.h"

#include <QPainter>
#include <QPainterPath>

#include "theme.h"

PushButton::PushButton(const QString &text, Type type, QWidget *parent)
    : QPushButton(text, parent)
    , m_type(type)
{
    QFont f = font();
    f.setPixelSize(Theme::instance()->fontPx(Tokens::FontSize::Body));
    setFont(f);
    setMinimumHeight(Theme::instance()->controlHeight(Tokens::ControlHeight::MD));

    // 悬浮/按下由自绘呈现,需要进出场时主动重绘
    setAttribute(Qt::WA_Hover, true);
    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

PushButton::PushButton(const QString &text, QWidget *parent)
    : PushButton(text, Type::Default, parent)
{
}

PushButton::Type PushButton::type() const
{
    return m_type;
}

void PushButton::setType(Type type)
{
    if(m_type == type)
    {
        return;
    }
    m_type = type;
    update();
}

void PushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    Theme *theme = Theme::instance();
    const bool disabled = !isEnabled();
    const bool hovered = underMouse();
    const bool pressed = isDown();

    QColor background;
    QColor foreground;
    QColor border;
    switch(m_type)
    {
    case Type::Primary:
        background = theme->color(Theme::Role::Primary);
        if(pressed)
        {
            background = theme->color(Theme::Role::PrimaryPressed);
        }
        else if(hovered)
        {
            background = theme->color(Theme::Role::PrimaryHover);
        }
        foreground = theme->color(Theme::Role::TextOnPrimary);
        break;
    case Type::Danger:
        background = theme->color(Theme::Role::Danger);
        if(pressed)
        {
            background = background.darker(120);
        }
        else if(hovered)
        {
            background = background.lighter(110);
        }
        foreground = theme->color(Theme::Role::TextOnPrimary);
        break;
    case Type::Text:
        // 无背景按钮须显式置透明:默认构造的 QColor 是不透明黑色,会画出黑块
        background = Qt::transparent;
        foreground = theme->color(Theme::Role::Text);
        if(hovered)
        {
            foreground = theme->color(Theme::Role::Primary);
        }
        if(pressed)
        {
            foreground = theme->color(Theme::Role::TextSecondary);
        }
        break;
    case Type::Default:
    default:
        background = theme->color(Theme::Role::Surface);
        if(pressed)
        {
            background = theme->color(Theme::Role::Border);
        }
        else if(hovered)
        {
            background = theme->color(Theme::Role::SurfaceVariant);
        }
        border = theme->color(hovered ? Theme::Role::BorderStrong
                                      : Theme::Role::Border);
        foreground = theme->color(Theme::Role::Text);
        break;
    }

    if(disabled)
    {
        switch(m_type)
        {
        case Type::Primary:
        case Type::Danger:
            background = theme->color(Theme::Role::SurfaceVariant);
            border = {};
            break;
        case Type::Text:
            background = Qt::transparent;
            break;
        case Type::Default:
        default:
            background = theme->color(Theme::Role::Surface);
            border = theme->color(Theme::Role::Border);
            break;
        }
        foreground = theme->color(Theme::Role::TextDisabled);
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const int radius = theme->radius(Tokens::Radius::MD);
    const QRect card = rect().adjusted(0, 0, -1, -1);

    if(background.alpha() > 0)
    {
        painter.setPen(Qt::NoPen);
        painter.setBrush(background);
        painter.drawRoundedRect(card, radius, radius);
    }
    if(border.isValid())
    {
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(border, 1));
        painter.drawRoundedRect(card, radius, radius);
    }

    painter.setPen(foreground);
    painter.setFont(font());
    painter.drawText(rect(), Qt::AlignCenter, text());
}
