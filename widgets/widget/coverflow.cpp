#include "coverflow.h"

#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QWheelEvent>

#include "theme.h"

namespace {

constexpr int kSideVisible = 2;     // 每侧最多画几张
constexpr qreal kSideScale = 0.66;  // 两侧封面的缩放
constexpr qreal kSpacingRatio = 0.62; // 相邻封面中心的间距 / 封面宽
constexpr qreal kReflectRatio = 0.35; // 倒影高度占封面比例
constexpr qreal kReflectOpacity = 0.22;

}

CoverFlow::CoverFlow(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    m_anim.setDuration(Theme::instance()->duration(Tokens::Duration::Normal));
    connect(&m_anim, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        setVisualPosition(value.toReal());
    });
    connect(Theme::instance(), &Theme::themeChanged, this, [this]() { update(); });
}

void CoverFlow::addCover(const QPixmap &cover)
{
    m_covers.append(cover);
    if(m_current < 0)
    {
        m_current = 0; // 首张自动选中(此时还没有监听者,不发信号)
        setVisualPosition(0);
    }
    update();
}

void CoverFlow::setCovers(const QVector<QPixmap> &covers)
{
    m_covers = covers;
    m_cache.clear();  // 同数量也可能是不同图,缓存整体失效
    m_current = covers.isEmpty() ? -1 : 0;
    m_anim.stop();
    setVisualPosition(m_current);
    update();
}

int CoverFlow::count() const
{
    return m_covers.size();
}

int CoverFlow::currentIndex() const
{
    return m_current;
}

void CoverFlow::setCurrentIndex(int index)
{
    if(index < 0 || index >= m_covers.size() || index == m_current)
    {
        return;
    }
    m_current = index;
    animateTo(index);
    emit currentChanged(index);
}

qreal CoverFlow::position() const
{
    return m_visualPos;
}

QSize CoverFlow::sizeHint() const
{
    return QSize(480, 220);
}

void CoverFlow::animateTo(int index)
{
    m_anim.stop();
    m_anim.setStartValue(m_visualPos);
    m_anim.setEndValue(qreal(index));
    m_anim.start();
}

void CoverFlow::setVisualPosition(qreal position)
{
    m_visualPos = position;
    update();
}

void CoverFlow::ensureCache(const QSize &base)
{
    // 基准尺寸(中间封面大小)变了整体重建;封面增删只补缺失项
    if(m_cacheBase != base)
    {
        m_cache.clear();
        m_cacheBase = base;
    }
    if(m_cache.size() == m_covers.size())
    {
        return;
    }
    m_cache.resize(m_covers.size());
    for(int i = 0; i < m_covers.size(); ++i)
    {
        if(m_cache.at(i).isNull() && !m_covers.at(i).isNull())
        {
            m_cache[i] = m_covers.at(i).scaled(base.width(), base.height(),
                                               Qt::KeepAspectRatio,
                                               Qt::SmoothTransformation);
        }
    }
}

void CoverFlow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    // 封面从缓存基准尺寸缩放绘制,开平滑插值保证两侧缩小仍有质感
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(rect(), Theme::instance()->color(Theme::Role::Background));
    if(m_covers.isEmpty())
    {
        return;
    }

    // 可见范围:position ± (kSideVisible + 1),留过渡余量
    const int first = qMax(0, int(m_visualPos) - kSideVisible - 1);
    const int last = qMin(m_covers.size() - 1, int(m_visualPos) + kSideVisible + 1);

    // 中间封面的基准尺寸:控件高度的上半部分(给倒影留出下方空间)
    const qreal coverHeight = height() / (1.0 + kReflectRatio) * 0.92;
    const qreal coverWidth = coverHeight;             // 按正方形封面处理
    const qreal spacing = coverWidth * kSpacingRatio;
    const QPointF center(width() / 2.0, height() / (1.0 + kReflectRatio) / 2.0 + 8);

    // 平滑缩放每张封面只在基准尺寸变化时做一次,绘制帧内只做缩放绘制
    ensureCache(QSize(qRound(coverWidth), qRound(coverHeight)));

    // 远 -> 近绘制,中间的封面最后画、盖在最上面
    QVector<int> order;
    for(int i = first; i <= last; ++i)
    {
        order.append(i);
    }
    std::sort(order.begin(), order.end(), [this](int a, int b) {
        return qAbs(a - m_visualPos) > qAbs(b - m_visualPos);
    });

    for(int i : order)
    {
        const QPixmap &base = m_cache.at(i);
        if(base.isNull())
        {
            continue;
        }

        const qreal dist = i - m_visualPos; // 负在左,正在右

        // 尺度:|dist|<=1 为原大小,到 |dist|=3 收缩到 kSideScale
        const qreal t = qBound(0.0, (qAbs(dist) - 1.0) / 2.0, 1.0);
        const qreal scale = 1.0 - t * (1.0 - kSideScale);
        const qreal w = base.width() * scale;
        const qreal h = base.height() * scale;
        const qreal x = center.x() + dist * spacing - w / 2.0;
        const qreal y = center.y() - h / 2.0;

        // 远处封面渐隐
        const qreal opacity = 1.0 - qBound(0.0, qAbs(dist) - 0.6, 2.4) * 0.30;
        const QRectF coverRect(x, y, w, h);

        // 倒影:painter 纵向翻转 + 压缩直接画(免去逐帧镜像拷贝),
        // 再用背景色纵向渐变盖出向下淡出的效果
        const QColor bg = Theme::instance()->color(Theme::Role::Background);
        const QRectF mirrorRect(x, coverRect.bottom() + 4.0, w, h * kReflectRatio);
        painter.save();
        painter.setOpacity(kReflectOpacity * opacity);
        painter.translate(0.0, mirrorRect.top() + mirrorRect.bottom());
        painter.scale(1.0, -1.0);
        painter.drawPixmap(mirrorRect, base,
                           QRectF(0, 0, base.width(), base.height()));
        painter.restore();

        QLinearGradient fade(mirrorRect.topLeft(), mirrorRect.bottomLeft());
        fade.setColorAt(0.0, QColor(bg.red(), bg.green(), bg.blue(), 0));
        fade.setColorAt(1.0, QColor(bg.red(), bg.green(), bg.blue(), 255));
        painter.setOpacity(1.0);
        painter.fillRect(mirrorRect, fade);

        painter.setOpacity(opacity);
        painter.drawPixmap(coverRect, base,
                           QRectF(0, 0, base.width(), base.height()));
        painter.setPen(QPen(Theme::instance()->color(Theme::Role::Border), 1));
        painter.drawRect(coverRect.adjusted(0, 0, -1, -1));
        painter.setOpacity(1.0);
    }
}

void CoverFlow::wheelEvent(QWheelEvent *event)
{
    const int steps = event->angleDelta().y() > 0 ? -1 : 1; // 下滚 = 下一张
    setCurrentIndex(qBound(0, m_current + steps, m_covers.size() - 1));
    event->accept();
}

void CoverFlow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
    case Qt::Key_Left:
        setCurrentIndex(qMax(0, m_current - 1));
        event->accept();
        break;
    case Qt::Key_Right:
        setCurrentIndex(qMin(m_covers.size() - 1, m_current + 1));
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

void CoverFlow::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_dragging = true;
        m_dragStartX = event->pos().x();
        m_dragStartPos = m_visualPos;
        m_anim.stop();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void CoverFlow::mouseMoveEvent(QMouseEvent *event)
{
    if(m_dragging)
    {
        // 向左拖 = 浏览后面的封面
        const qreal spacing = height() * kSpacingRatio;
        const qreal raw = m_dragStartPos - (event->pos().x() - m_dragStartX) / spacing;
        setVisualPosition(qBound(0.0, raw, qreal(m_covers.size() - 1)));
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void CoverFlow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton && m_dragging)
    {
        m_dragging = false;
        // 吸附到最近的封面
        const int target = qRound(qBound(0.0, m_visualPos, qreal(m_covers.size() - 1)));
        if(target != m_current)
        {
            m_current = target;
            emit currentChanged(target);
        }
        animateTo(target);
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}
