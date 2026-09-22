#pragma once

#include <QPixmap>
#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

// 封面流(CoverFlow):横向浏览封面——中间大、两侧渐小渐淡,带倒影;
// 滚轮 / 左右键切换,按住拖动翻页,滑动动效归位;发 currentChanged(index)。
// 用于封面浏览页
class CoverFlow : public QWidget
{
    Q_OBJECT
public:
    explicit CoverFlow(QWidget *parent = nullptr);

    void addCover(const QPixmap &cover);                 // 追加一张
    void setCovers(const QVector<QPixmap> &covers);      // 替换全部,索引复位
    int count() const;
    int currentIndex() const;
    void setCurrentIndex(int index);                     // 动画滑到 index

    qreal position() const; // 当前滑动位置(浮点索引,暴露给测试)

    QSize sizeHint() const override;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void animateTo(int index);
    void setVisualPosition(qreal position);
    void ensureCache(const QSize &base);  // 基准尺寸不符时把封面平滑缩放缓存一遍

    QVector<QPixmap> m_covers;
    int m_current = -1;
    qreal m_visualPos = 0;
    qreal m_dragStartX = 0;
    qreal m_dragStartPos = 0;
    bool m_dragging = false;
    QVariantAnimation m_anim;
    QVector<QPixmap> m_cache;   // 每张封面按基准尺寸(中间封面大小)缩放一次的缓存
    QSize m_cacheBase;
};
