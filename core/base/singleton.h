#pragma once

// 简单的 CRTP 单例基类：class Foo : public Singleton<Foo> {};
template <typename T>
class Singleton
{
public:
    static T *instance()
    {
        static T inst;
        return &inst;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton &) = delete;
    Singleton &operator=(const Singleton &) = delete;
};

common\widgets\CMakeLists.txt
add_library(widgets STATIC
                framelesswidget.h
                    framelesswidget.cpp)
    add_library(canfan::widgets ALIAS widgets)
        target_link_libraries(widgets PUBLIC Qt6::Widgets canfan::core)
            target_include_directories(widgets PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

                common\widgets\framelesswidget.h —— 对应 TTK 的 TTKAbstractMoveWidget
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

common\widgets\framelesswidget.cpp
#include "framelesswidget.h"

#include <QMouseEvent>

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
}

void FramelessWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_pressed = true;
        m_pressPos = event->globalPosition().toPoint();
    }
    QWidget::mousePressEvent(event);
}

void FramelessWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_pressed)
    {
        const QPoint offset = event->globalPosition().toPoint() - m_pressPos;
        window()->move(window()->pos() + offset);
        m_pressPos = event->globalPosition().toPoint();
    }
    QWidget::mouseMoveEvent(event);
}

void FramelessWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressed = false;
    QWidget::mouseReleaseEvent(event);
}
