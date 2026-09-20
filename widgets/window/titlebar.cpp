#include "titlebar.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>

#include "framelesshandler.h"
#include "theme.h"

namespace {

constexpr int kBarButtonWidth = 40;
constexpr int kBarHeight = 32;

// 标题栏按钮:纯绘制的最小化/最大化(还原)/关闭,不使用图标资源
class CaptionButton : public QWidget
{
    Q_OBJECT
public:
    enum class Kind
    {
        Minimize,
        Maximize,
        Restore,
        Close
    };

    explicit CaptionButton(Kind kind, QWidget *parent = nullptr)
        : QWidget(parent)
        , m_kind(kind)
    {
        setFixedSize(kBarButtonWidth, kBarHeight);
        setCursor(Qt::ArrowCursor);
        setAttribute(Qt::WA_Hover, true);
    }

    Kind kind() const { return m_kind; }
    void setKind(Kind kind)
    {
        if(m_kind != kind)
        {
            m_kind = kind;
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)
        Theme *theme = Theme::instance();
        const bool closeHover = m_kind == Kind::Close && underMouse();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        if(underMouse())
        {
            // 关闭悬停为危险红实底,其余为次级表面
            painter.setPen(Qt::NoPen);
            painter.setBrush(closeHover ? theme->color(Theme::Role::Danger)
                                        : theme->color(Theme::Role::SurfaceVariant));
            painter.drawRect(rect());
        }

        // 图形全部用画笔画线/描边,1.2px 在浅底深字下清晰且不显糙
        QColor glyph = closeHover ? theme->color(Theme::Role::TextOnPrimary)
                                  : theme->color(Theme::Role::Text);
        QPen pen(glyph, 1.2);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);

        const QPointF c = rect().center();
        switch(m_kind)
        {
        case Kind::Minimize:
            painter.drawLine(QPointF(c.x() - 5, c.y()), QPointF(c.x() + 5, c.y()));
            break;
        case Kind::Maximize:
            painter.drawRect(QRectF(c.x() - 5, c.y() - 5, 10, 10));
            break;
        case Kind::Restore:
            // 前后错位的两个方框
            painter.drawRect(QRectF(c.x() - 5, c.y() - 2, 8, 8));
            painter.drawRect(QRectF(c.x() - 2, c.y() - 5, 8, 8));
            break;
        case Kind::Close:
            painter.drawLine(QPointF(c.x() - 4, c.y() - 4), QPointF(c.x() + 4, c.y() + 4));
            painter.drawLine(QPointF(c.x() - 4, c.y() + 4), QPointF(c.x() + 4, c.y() - 4));
            break;
        }
    }

    void mousePressEvent(QMouseEvent *event) override
    {
        // 同 ToggleSwitch:显式接收按下,才能拿到 release
        if(event->button() == Qt::LeftButton)
        {
            event->accept();
            update();
            return;
        }
        QWidget::mousePressEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent *event) override
    {
        if(event->button() == Qt::LeftButton && rect().contains(event->pos()))
        {
            emit clicked();
            event->accept();
            update();
            return;
        }
        QWidget::mouseReleaseEvent(event);
    }

signals:
    void clicked();

private:
    Kind m_kind;
};

} // namespace

TitleBar::TitleBar(QWidget *window, const QString &title)
    : QWidget(window)
    , m_window(window)
{
    // 标题栏纳入窗口拖动体系:面板上的按下/移动/双击按窗口坐标处理
    // (拖动、边缘拉伸、双击最大化),事件被截断不再冒泡到窗口;
    // 按钮是独立子控件,自己消费点击,不经过这里
    auto *drag = new FramelessHandler(m_window, this, false);
    drag->watch(this);

    setFixedHeight(kBarHeight);

    m_title = new QLabel(title, this);
    QFont titleFont = m_title->font();
    titleFont.setPixelSize(Theme::instance()->fontPx(Tokens::FontSize::BodyStrong));
    m_title->setFont(titleFont);
    m_title->setStyleSheet(QStringLiteral("background:transparent;"));

    m_minButton = new CaptionButton(CaptionButton::Kind::Minimize, this);
    m_minButton->setObjectName(QStringLiteral("minButton"));
    m_maxButton = new CaptionButton(CaptionButton::Kind::Maximize, this);
    m_maxButton->setObjectName(QStringLiteral("maxButton"));
    m_closeButton = new CaptionButton(CaptionButton::Kind::Close, this);
    m_closeButton->setObjectName(QStringLiteral("closeButton"));

    // 按钮类只在本文件可见,向下转型安全
    connect(static_cast<CaptionButton *>(m_minButton), &CaptionButton::clicked,
            m_window, &QWidget::showMinimized);
    connect(static_cast<CaptionButton *>(m_maxButton), &CaptionButton::clicked, this, [this]() {
        m_window->isMaximized() ? m_window->showNormal() : m_window->showMaximized();
    });
    connect(static_cast<CaptionButton *>(m_closeButton), &CaptionButton::clicked, this, [this]() {
        emit closeRequested();
        m_window->close();
    });

    // 窗口最大化状态变化时切换 最大化/还原 图形
    m_window->installEventFilter(this);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_title);
    layout->addStretch();
    layout->addWidget(m_minButton);
    layout->addWidget(m_maxButton);
    layout->addWidget(m_closeButton);

    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

void TitleBar::setTitle(const QString &title)
{
    m_title->setText(title);
}

QString TitleBar::title() const
{
    return m_title->text();
}

void TitleBar::setClosable(bool closable)
{
    m_closeButton->setVisible(closable);
}

bool TitleBar::isClosable() const
{
    return m_closeButton->isVisible();
}

QSize TitleBar::sizeHint() const
{
    return QSize(200, kBarHeight);
}

void TitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    // 不铺底色:窗口的圆角卡片供底,铺满到边的底色会把顶角盖成方角
    // (v0.12.0 起窗口自带阴影 + 圆角)。只画底部分隔线,亮暗跟随主题
    QPainter painter(this);
    painter.setPen(QPen(Theme::instance()->color(Theme::Role::Border), 1));
    painter.drawLine(0, height() - 1, width(), height() - 1);

    m_title->setStyleSheet(QStringLiteral("background:transparent;color:%1;")
                               .arg(Theme::instance()->color(Theme::Role::Text).name()));
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == m_window && event->type() == QEvent::WindowStateChange)
    {
        static_cast<CaptionButton *>(m_maxButton)->setKind(
            m_window->isMaximized() ? CaptionButton::Kind::Restore
                                    : CaptionButton::Kind::Maximize);
    }
    return QWidget::eventFilter(watched, event);
}

#include "titlebar.moc"
