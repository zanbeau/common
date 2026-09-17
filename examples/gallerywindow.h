#pragma once

#include <QWidget>

class QListWidget;
class QStackedWidget;

// 控件浏览器(仿 TTKExample):左侧分类列表,右侧示例页
// 接入新示例页:addPage("名称", new XxxPage)
class GalleryWindow : public QWidget
{
    Q_OBJECT
public:
    explicit GalleryWindow(QWidget *parent = nullptr);

    void addPage(const QString &name, QWidget *page);

private:
    QListWidget *m_list = nullptr;
    QStackedWidget *m_stack = nullptr;
};
