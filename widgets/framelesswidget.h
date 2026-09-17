#pragma once

#include <QWidget>

// 无边框窗口基类：按住客户区拖动窗口，靠近窗口边缘时可拖拽缩放
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
    enum class ResizeDirection
    {
        None,
        Top, Bottom, Left, Right,
        TopLeft, TopRight, BottomLeft, BottomRight
    };

    ResizeDirection resizeDirectionAt(const QPoint &pos) const;
    void updateResizeCursor(const QPoint &pos);
    QRect resizedGeometry(const QPoint &globalPos) const;

    bool m_pressed = false;
    QPoint m_pressPos;

    bool m_resizing = false;
    ResizeDirection m_direction = ResizeDirection::None;
    QPoint m_resizeStartPos;
    QRect m_resizeStartGeometry;

    static constexpr int ResizeMargin = 6;
};
