#pragma once

#include <QWidget>

class QPushButton;
class WaitSpinner;

class WaitSpinnerPage : public QWidget
{
    Q_OBJECT
public:
    explicit WaitSpinnerPage(QWidget *parent = nullptr);

private:
    WaitSpinner *m_spinner = nullptr;
    QPushButton *m_toggle = nullptr;
};
