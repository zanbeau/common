#include "framelessdialog.h"

#include "framelesshandler.h"

FramelessDialog::FramelessDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    m_handler = new FramelessHandler(this, this);
}

void FramelessDialog::setResizeMargin(int margin)
{
    m_handler->setResizeMargin(margin);
}

int FramelessDialog::resizeMargin() const
{
    return m_handler->resizeMargin();
}
