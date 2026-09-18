#pragma once

#include <QWidget>

class MarqueeLabel;
class QPushButton;

class MarqueePage : public QWidget
{
    Q_OBJECT
public:
    explicit MarqueePage(QWidget *parent = nullptr);

private:
    MarqueeLabel *m_label = nullptr;
    QPushButton *m_toggle = nullptr;
};
