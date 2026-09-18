#pragma once

#include <QWidget>

class QLabel;
class QSlider;

class ClickedSliderPage : public QWidget
{
    Q_OBJECT
public:
    explicit ClickedSliderPage(QWidget *parent = nullptr);

private:
    QSlider *m_slider = nullptr;
    QLabel *m_value = nullptr;
};
