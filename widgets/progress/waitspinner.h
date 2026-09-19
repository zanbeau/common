#pragma once

#include <QColor>
#include <QVariantAnimation>
#include <QWidget>

// 不定进度的转圈指示(旋转弧线):start()/stop() 控制,
// 转一圈时长、颜色、线宽可调;停止时不绘制。
// 默认颜色取 Theme 主色(亮暗联动),setColor() 设过自定义色后不再跟随主题
class WaitSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit WaitSpinner(QWidget *parent = nullptr);

    bool isSpinning() const;

    void setLoopDuration(int ms);  // 旋转一周的时长,<=0 视为 1200
    int loopDuration() const;
    void setColor(const QColor &color);
    QColor color() const;
    void setLineWidth(int width);
    int lineWidth() const;

    // 当前旋转角度(0..360),主要供测试观察
    int angle() const;

public slots:
    void start();
    void stop();

protected:
    QSize sizeHint() const override;
    void paintEvent(QPaintEvent *event) override;

private:
    int m_loopDuration = 1200;
    int m_lineWidth = 3;
    QColor m_color;
    bool m_customColor = false;
    QVariantAnimation m_angle;
};
