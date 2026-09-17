#pragma once

#include <QWidget>

// 无边框窗口基类：按住客户区任意位置可拖动窗口
class FramelessWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FramelessWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_pressed = false;
    QPoint m_pressPos;
};
