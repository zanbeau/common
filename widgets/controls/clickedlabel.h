#pragma once

#include <QLabel>

// 可点击的 QLabel:左键在标签内按下并释放时发出 clicked()
// 按下事件会被拦截不再向上传播,适合放进无边框窗口的标题栏
class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickedLabel(const QString &text = {}, QWidget *parent = nullptr);

signals:
    void clicked();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_pressed = false;
};
