#pragma once

#include <QWidget>

class AnimationStackedWidget;
class QPushButton;

class AnimationStackedPage : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationStackedPage(QWidget *parent = nullptr);

private slots:
    void switchPage();

private:
    AnimationStackedWidget *m_stack = nullptr;
    QPushButton *m_next = nullptr;
};
