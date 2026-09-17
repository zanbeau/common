#include "gallerywindow.h"

#include <QHBoxLayout>
#include <QListWidget>
#include <QStackedWidget>

GalleryWindow::GalleryWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("common 控件浏览器"));
    resize(760, 480);

    m_list = new QListWidget;
    m_list->setFixedWidth(150);
    m_stack = new QStackedWidget;

    connect(m_list, &QListWidget::currentRowChanged,
            m_stack, &QStackedWidget::setCurrentIndex);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);
    layout->addWidget(m_list);
    layout->addWidget(m_stack, 1);
}

void GalleryWindow::addPage(const QString &name, QWidget *page)
{
    m_list->addItem(name);
    m_stack->addWidget(page);
    if(m_list->count() == 1)
        m_list->setCurrentRow(0);
}
