#include "playlisttable.h"

#include <QEvent>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>

#include "duration.h"
#include "iconbutton.h"
#include "theme.h"

namespace {

constexpr int kDefaultRowHeight = 44;
constexpr int kMinRowHeight = 28;
constexpr int kIndexColWidth = 40;   // 序号列(播放中该列换播放字形)
constexpr int kTextLeftMargin = 12;  // 文本块与左侧(封面/序号列)的间距
constexpr int kRightMargin = 14;     // 时长列右侧留白

} // namespace

PlaylistTable::PlaylistTable(QWidget *parent)
    : QAbstractScrollArea(parent)
{
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::StrongFocus);
    viewport()->setMouseTracking(true);
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setSingleStep(kDefaultRowHeight);

    connect(verticalScrollBar(), &QAbstractSlider::valueChanged,
            viewport(), qOverload<>(&QWidget::update));
    connect(Theme::instance(), &Theme::themeChanged,
            viewport(), qOverload<>(&QWidget::update));
}

void PlaylistTable::setTracks(const QVector<Track> &tracks)
{
    m_tracks = tracks;
    m_thumbs.clear();
    m_thumbs.resize(tracks.size());
    m_hasCovers = false;
    for(const Track &track : tracks)
    {
        if(!track.cover.isNull())
        {
            m_hasCovers = true;
            break;
        }
    }
    m_current = -1;
    m_playing = -1;
    m_hover = -1;
    updateScrollRange();
    viewport()->update();
}

int PlaylistTable::count() const
{
    return m_tracks.size();
}

PlaylistTable::Track PlaylistTable::track(int index) const
{
    if(index < 0 || index >= m_tracks.size())
    {
        return Track();
    }
    return m_tracks.at(index);
}

int PlaylistTable::currentIndex() const
{
    return m_current;
}

void PlaylistTable::setCurrentIndex(int index)
{
    if(index >= m_tracks.size())
    {
        index = -1;
    }
    if(m_current == index)
    {
        return;
    }
    m_current = index;
    if(index >= 0)
    {
        ensureVisible(index);
    }
    viewport()->update();
    emit currentChanged(index);
}

int PlaylistTable::playingIndex() const
{
    return m_playing;
}

void PlaylistTable::setPlayingIndex(int index)
{
    if(index >= m_tracks.size())
    {
        index = -1;
    }
    if(m_playing == index)
    {
        return;
    }
    m_playing = index;
    viewport()->update();
}

int PlaylistTable::rowHeight() const
{
    return m_rowHeight;
}

void PlaylistTable::setRowHeight(int height)
{
    const int clamped = qMax(height, kMinRowHeight);
    if(m_rowHeight == clamped)
    {
        return;
    }
    m_rowHeight = clamped;
    m_thumbs.clear();
    m_thumbs.resize(m_tracks.size());
    verticalScrollBar()->setSingleStep(clamped);
    updateScrollRange();
    viewport()->update();
}

QSize PlaylistTable::sizeHint() const
{
    return QSize(420, 320);
}

void PlaylistTable::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(viewport());
    Theme *theme = Theme::instance();
    painter.fillRect(viewport()->rect(), theme->color(Theme::Role::Surface));

    if(m_tracks.isEmpty())
    {
        painter.setPen(theme->color(Theme::Role::TextTertiary));
        painter.drawText(viewport()->rect(), Qt::AlignCenter, QStringLiteral("列表为空"));
        return;
    }

    // 虚拟化:只画视口覆盖到的行
    const int offset = verticalScrollBar()->value();
    const int first = qMax(0, offset / m_rowHeight);
    const int last = qMin(m_tracks.size() - 1,
                          (offset + viewport()->height() + m_rowHeight - 1) / m_rowHeight);

    QFont titleFont = painter.font();
    titleFont.setPixelSize(theme->fontPx(Tokens::FontSize::Body));
    QFont artistFont = painter.font();
    artistFont.setPixelSize(theme->fontPx(Tokens::FontSize::Caption));

    for(int i = first; i <= last; ++i)
    {
        const QRect row = rowRect(i);
        const bool playing = (i == m_playing);

        // 行底:播放 > 选中 > 悬浮
        QColor rowBg;
        if(playing)
        {
            rowBg = theme->color(Theme::Role::Primary);
            rowBg.setAlpha(38);
        }
        else if(i == m_current)
        {
            rowBg = theme->color(Theme::Role::Primary);
            rowBg.setAlpha(22);
        }
        else if(i == m_hover)
        {
            rowBg = theme->color(Theme::Role::SurfaceVariant);
        }
        if(rowBg.isValid())
        {
            painter.fillRect(row, rowBg);
        }
        if(playing)
        {
            painter.fillRect(row.left(), row.top(), 3, row.height(),
                             theme->color(Theme::Role::Primary));
        }

        // 左列:播放字形或序号
        const QRect indexRect(row.left(), row.top(), kIndexColWidth, row.height());
        if(playing)
        {
            const QColor glyphColor = theme->color(Theme::Role::Primary);
            IconButton::paintGlyph(&painter, IconButton::Glyph::Play,
                                   QRect(indexRect.center() - QPoint(7, 7),
                                         indexRect.center() + QPoint(7, 8)),
                                   glyphColor);
        }
        else
        {
            painter.setPen(theme->color(i == m_current || i == m_hover
                                            ? Theme::Role::TextSecondary
                                            : Theme::Role::TextTertiary));
            painter.drawText(indexRect, Qt::AlignCenter, QString::number(i + 1));
        }

        // 封面缩略(列表里存在任一封面才启用该列)
        int textLeft = row.left() + kIndexColWidth;
        if(m_hasCovers)
        {
            const int side = m_rowHeight - 12;
            const QRect coverRect(row.left() + kIndexColWidth + kTextLeftMargin,
                                  row.top() + (row.height() - side) / 2, side, side);
            const QPixmap cover = thumb(i);
            if(!cover.isNull())
            {
                QPainterPath rounded;
                rounded.addRoundedRect(coverRect, theme->radius(Tokens::Radius::SM),
                                       theme->radius(Tokens::Radius::SM));
                painter.save();
                painter.setClipPath(rounded);
                painter.setRenderHint(QPainter::SmoothPixmapTransform);
                painter.drawPixmap(coverRect, cover);
                painter.restore();
            }
            textLeft = coverRect.right() + kTextLeftMargin;
        }

        // 标题 + 艺术家两行;时长右对齐
        const QString duration = Duration::format(m_tracks.at(i).duration);
        const int durationWidth = painter.fontMetrics().horizontalAdvance(duration);
        const QRect textRect(textLeft, row.top(),
                             row.right() - kRightMargin - durationWidth - kTextLeftMargin
                                 - textLeft,
                             row.height());
        painter.setFont(titleFont);
        painter.setPen(theme->color(playing ? Theme::Role::Primary : Theme::Role::Text));
        painter.drawText(QRect(textRect.left(), row.top() + 5, textRect.width(), 18),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         QFontMetrics(titleFont).elidedText(m_tracks.at(i).title,
                                                            Qt::ElideRight,
                                                            textRect.width()));
        painter.setFont(artistFont);
        painter.setPen(theme->color(Theme::Role::TextSecondary));
        painter.drawText(QRect(textRect.left(), row.bottom() - 21, textRect.width(), 16),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         QFontMetrics(artistFont).elidedText(m_tracks.at(i).artist,
                                                             Qt::ElideRight,
                                                             textRect.width()));
        painter.setPen(theme->color(Theme::Role::TextSecondary));
        painter.drawText(QRect(row.right() - kRightMargin - durationWidth,
                               row.top(), durationWidth, row.height()),
                         Qt::AlignVCenter | Qt::AlignRight, duration);
    }
}

void PlaylistTable::mouseMoveEvent(QMouseEvent *event)
{
    const int row = rowAt(event->pos());
    if(row != m_hover)
    {
        m_hover = row;
        viewport()->update();
    }
    QAbstractScrollArea::mouseMoveEvent(event);
}

void PlaylistTable::leaveEvent(QEvent *event)
{
    if(m_hover != -1)
    {
        m_hover = -1;
        viewport()->update();
    }
    QAbstractScrollArea::leaveEvent(event);
}

void PlaylistTable::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        const int row = rowAt(event->pos());
        if(row >= 0)
        {
            setCurrentIndex(row);
            return;
        }
    }
    QAbstractScrollArea::mousePressEvent(event);
}

void PlaylistTable::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        const int row = rowAt(event->pos());
        if(row >= 0)
        {
            setCurrentIndex(row);
            emit activated(row);
            return;
        }
    }
    QAbstractScrollArea::mouseDoubleClickEvent(event);
}

void PlaylistTable::keyPressEvent(QKeyEvent *event)
{
    if(m_tracks.isEmpty())
    {
        QAbstractScrollArea::keyPressEvent(event);
        return;
    }

    const int page = qMax(1, viewport()->height() / m_rowHeight);
    switch(event->key())
    {
    case Qt::Key_Up:
    case Qt::Key_Down:
    {
        const int delta = event->key() == Qt::Key_Up ? -1 : 1;
        setCurrentIndex(qBound(0, qMax(m_current, 0) + delta, m_tracks.size() - 1));
        break;
    }
    case Qt::Key_PageUp:
    case Qt::Key_PageDown:
    {
        const int delta = event->key() == Qt::Key_PageUp ? -page : page;
        setCurrentIndex(qBound(0, qMax(m_current, 0) + delta, m_tracks.size() - 1));
        break;
    }
    case Qt::Key_Home:
        setCurrentIndex(0);
        break;
    case Qt::Key_End:
        setCurrentIndex(m_tracks.size() - 1);
        break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
        if(m_current >= 0)
        {
            emit activated(m_current);
        }
        break;
    default:
        QAbstractScrollArea::keyPressEvent(event);
        break;
    }
}

void PlaylistTable::resizeEvent(QResizeEvent *event)
{
    QAbstractScrollArea::resizeEvent(event);
    updateScrollRange();
}

int PlaylistTable::rowAt(const QPoint &pos) const
{
    if(pos.x() < 0 || pos.x() > viewport()->width())
    {
        return -1;
    }
    const int index = (pos.y() + verticalScrollBar()->value()) / m_rowHeight;
    if(index < 0 || index >= m_tracks.size() || pos.y() > viewport()->height())
    {
        return -1;
    }
    return index;
}

QRect PlaylistTable::rowRect(int index) const
{
    const int y = index * m_rowHeight - verticalScrollBar()->value();
    return QRect(0, y, viewport()->width(), m_rowHeight);
}

void PlaylistTable::ensureVisible(int index)
{
    QScrollBar *bar = verticalScrollBar();
    const int top = index * m_rowHeight;
    if(top < bar->value())
    {
        bar->setValue(top);
    }
    else if(top + m_rowHeight > bar->value() + viewport()->height())
    {
        bar->setValue(top + m_rowHeight - viewport()->height());
    }
}

void PlaylistTable::updateScrollRange()
{
    verticalScrollBar()->setRange(0, qMax(0, m_tracks.size() * m_rowHeight
                                                - viewport()->height()));
    verticalScrollBar()->setPageStep(qMax(1, viewport()->height()));
}

QPixmap PlaylistTable::thumb(int index) const
{
    const QPixmap &source = m_tracks.at(index).cover;
    if(source.isNull())
    {
        return QPixmap();
    }
    if(m_thumbs.at(index).isNull())
    {
        const int side = m_rowHeight - 12;
        m_thumbs[index] = source.scaled(side, side, Qt::KeepAspectRatioByExpanding,
                                        Qt::SmoothTransformation);
    }
    return m_thumbs.at(index);
}
