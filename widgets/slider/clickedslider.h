#pragma once

#include <QSlider>

// 点击任意位置手柄直接跳过去的 QSlider(音量条/进度条习惯),
// 跳转后按住不放仍可继续拖动;左键单击时额外发出 clicked()
class ClickedSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ClickedSlider(Qt::Orientation orientation, QWidget *parent = nullptr);
    explicit ClickedSlider(QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
};
