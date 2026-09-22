#pragma once

#include <QPixmap>
#include <QVariantAnimation>
#include <QWidget>

// 旋转封面 Label:图片绕中心持续旋转(唱片效果),可选圆形裁剪。
// 默认静止,setRunning(true) 开始转;速度一圈 loopDuration 毫秒
class RotateLabel : public QWidget
{
    Q_OBJECT
public:
    explicit RotateLabel(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    QPixmap pixmap() const;

    void setRunning(bool running);
    bool isRunning() const;

    void setLoopDuration(int ms); // 一圈的时长
    int loopDuration() const;

    void setCircular(bool circular); // 圆形裁剪(唱片)
    bool isCircular() const;

    int angle() const; // 当前角度(暴露给测试)

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_pixmap;
    QVariantAnimation m_angle;
    bool m_circular = false;
    QPixmap m_scaled;         // 平滑缩放缓存:只随原图/尺寸变化重算,旋转帧内复用
    int m_scaledExtent = -1;
};
