#include "framelesswidget.h"

#include <QPainter>

#include "framelesshandler.h"
#include "framelessshadow.h"

FramelessWidget::FramelessWidget(QWidget *parent)
    : QWidget(parent)
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    // 半透明窗口:窗口本体只画阴影与圆角卡片,内容画在容器上
    setAttribute(Qt::WA_TranslucentBackground);
    m_handler = new FramelessHandler(this, this);
    m_shadow = new FramelessShadow(this);
    m_handler->setClientWidget(m_shadow->contentWidget());
}

QWidget *FramelessWidget::contentWidget() const
{
    return m_shadow->contentWidget();
}

QVBoxLayout *FramelessWidget::contentLayout() const
{
    return m_shadow->contentLayout();
}

void FramelessWidget::setShadowEnabled(bool enabled)
{
    m_shadow->setShadowEnabled(enabled);
}

bool FramelessWidget::shadowEnabled() const
{
    return m_shadow->shadowEnabled();
}

int FramelessWidget::shadowMargin() const
{
    return m_shadow->shadowMargin();
}

void FramelessWidget::setResizeMargin(int margin)
{
    m_handler->setResizeMargin(margin);
}

int FramelessWidget::resizeMargin() const
{
    return m_handler->resizeMargin();
}

QSize FramelessWidget::minimumSizeHint() const
{
    const QSize hint = m_shadow->contentLayout()->minimumSize();
    const int m = m_shadow->shadowMargin();
    return QSize(hint.width() + 2 * m, hint.height() + 2 * m);
}

void FramelessWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    m_shadow->paint(&painter);
}
