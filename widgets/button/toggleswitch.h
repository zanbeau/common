#pragma once

#include <QVariantAnimation>
#include <QWidget>

// 开关(Ant Switch 风格):轨道 + 圆形滑块,点击/空格切换,
// 滑块位移带动效;颜色取自 Theme,亮暗自动联动
class ToggleSwitch : public QWidget
{
    Q_OBJECT
public:
    explicit ToggleSwitch(QWidget *parent = nullptr);

    bool isChecked() const;
    void setChecked(bool checked); // 状态不变时不发信号

    // 滑块位置(0=关 1=开),动画进行中介于两者之间(暴露给测试)
    qreal knobPos() const;

    QSize sizeHint() const override;

signals:
    void toggled(bool checked);
    void clicked();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    void toggle();

    bool m_checked = false;
    bool m_pressedInside = false;
    bool m_keyboardFocus = false;  // 焦点是否来自键盘导航,决定是否画焦点圈
    QVariantAnimation m_knob;
};
