#include "framelessdialog.h"

#include <QPainter>

#include "framelesshandler.h"
#include "framelessshadow.h"

FramelessDialog::FramelessDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    m_handler = new FramelessHandler(this, this);
    m_shadow = new FramelessShadow(this);
    m_handler->setClientWidget(m_shadow->contentWidget());
}

QWidget *FramelessDialog::contentWidget() const
{
    return m_shadow->contentWidget();
}

QVBoxLayout *FramelessDialog::contentLayout() const
{
    return m_shadow->contentLayout();
}

void FramelessDialog::setShadowEnabled(bool enabled)
{
    m_shadow->setShadowEnabled(enabled);
}

bool FramelessDialog::shadowEnabled() const
{
    return m_shadow->shadowEnabled();
}

int FramelessDialog::shadowMargin() const
{
    return m_shadow->shadowMargin();
}

void FramelessDialog::setResizeMargin(int margin)
{
    m_handler->setResizeMargin(margin);
}

int FramelessDialog::resizeMargin() const
{
    return m_handler->resizeMargin();
}

QSize FramelessDialog::minimumSizeHint() const
{
    const QSize hint = m_shadow->contentLayout()->minimumSize();
    const int m = m_shadow->shadowMargin();
    return QSize(hint.width() + 2 * m, hint.height() + 2 * m);
}

void FramelessDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    m_shadow->paint(&painter);
}
