#include "lyricsview.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include "theme.h"

namespace {

constexpr int kLineHeight = 44;   // 行高固定,当前行放大字号仍容纳得下
constexpr int kSideMargin = 24;   // 长行省略的左右留白

} // namespace

LyricsView::LyricsView(QWidget *parent)
    : QWidget(parent)
{
    m_scroll.setDuration(Theme::instance()->duration(Tokens::Duration::Normal));
    m_scroll.setEasingCurve(QEasingCurve::OutCubic);
    connect(&m_scroll, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_offset = value.toReal();
        update();
    });
    connect(Theme::instance(), &Theme::themeChanged, this, qOverload<>(&QWidget::update));
}

void LyricsView::setLines(const QVector<LrcLine> &lines)
{
    m_lines = lines;
    m_current = -1;
    m_offset = 0;
    m_scroll.stop();
    update();
}

void LyricsView::clear()
{
    setLines({});
}

int LyricsView::lineCount() const
{
    return m_lines.size();
}

int LyricsView::currentLine() const
{
    return m_current;
}

qint64 LyricsView::lineTime(int index) const
{
    if(index < 0 || index >= m_lines.size())
    {
        return -1;
    }
    return m_lines.at(index).ms;
}

QSize LyricsView::sizeHint() const
{
    return QSize(360, 320);
}

void LyricsView::setTime(qint64 ms)
{
    // 行序已升序:线性扫出 <= ms 的最后一行(歌词行数有限,全量扫代价可忽略)
    int index = -1;
    for(int i = 0; i < m_lines.size() && m_lines.at(i).ms <= ms; ++i)
    {
        index = i;
    }
    if(index != m_current)
    {
        m_current = index;
        if(index >= 0)
        {
            scrollTo(index);   // 行切换才居中:不与滚轮浏览打架
        }
        update();
    }
}

void LyricsView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    Theme *theme = Theme::instance();

    if(m_lines.isEmpty())
    {
        painter.setPen(theme->color(Theme::Role::TextTertiary));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无歌词"));
        return;
    }

    QFont normalFont = painter.font();
    normalFont.setPixelSize(theme->fontPx(Tokens::FontSize::Body));
    QFont currentFont = normalFont;
    currentFont.setPixelSize(theme->fontPx(Tokens::FontSize::Subtitle));
    currentFont.setBold(true);

    // 虚拟化:只画视口覆盖到的行
    const int first = qMax(0, int(m_offset) / kLineHeight);
    const int last = qMin(m_lines.size() - 1,
                          int(m_offset + height()) / kLineHeight + 1);
    for(int i = first; i <= last; ++i)
    {
        const QRect lineRect(0, i * kLineHeight - int(m_offset), width(), kLineHeight);
        const bool current = (i == m_current);

        // 当前行主文本色 + 放大,其余淡色;长行居中省略
        painter.setFont(current ? currentFont : normalFont);
        painter.setPen(theme->color(current ? Theme::Role::Text
                                            : Theme::Role::TextTertiary));
        const QString text = QFontMetrics(current ? currentFont : normalFont)
                                 .elidedText(m_lines.at(i).text, Qt::ElideRight,
                                             width() - 2 * kSideMargin);
        painter.drawText(lineRect, Qt::AlignHCenter | Qt::AlignVCenter, text);
    }
}

void LyricsView::mousePressEvent(QMouseEvent *event)
{
    const int index = lineAt(event->pos());
    if(index >= 0 && !m_lines.at(index).text.isEmpty())
    {
        emit lineClicked(m_lines.at(index).ms);
    }
    QWidget::mousePressEvent(event);
}

void LyricsView::wheelEvent(QWheelEvent *event)
{
    // 自由浏览:滚轮平移;行切换时 setTime 会重新动画居中
    m_scroll.stop();
    const int delta = event->angleDelta().y();
    if(delta != 0)
    {
        m_offset = qBound<qreal>(0.0, m_offset - delta / 2.0, qreal(maxOffset()));
        update();
        event->accept();
        return;
    }
    QWidget::wheelEvent(event);
}

int LyricsView::lineAt(const QPoint &pos) const
{
    const int index = (pos.y() + int(m_offset)) / kLineHeight;
    if(index < 0 || index >= m_lines.size() || pos.y() > height())
    {
        return -1;
    }
    return index;
}

int LyricsView::maxOffset() const
{
    return qMax(0, m_lines.size() * kLineHeight - height());
}

void LyricsView::scrollTo(int index)
{
    m_scroll.stop();
    const qreal target = qBound<qreal>(0.0,
                                       index * kLineHeight + kLineHeight / 2.0 - height() / 2.0,
                                       qreal(maxOffset()));
    m_scroll.setStartValue(m_offset);
    m_scroll.setEndValue(target);
    m_scroll.start();
}
