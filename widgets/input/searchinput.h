#pragma once

#include <QLineEdit>

class QAction;

// 搜索输入框:前置放大镜图标 + 内置清除按钮,回车发出 searchRequested()。
// 图标随主题重绘,占位文案同 QLineEdit 用法
class SearchInput : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchInput(const QString &placeholder = {}, QWidget *parent = nullptr);

    // 前置图标对应的 action(可换成自定义图标)
    QAction *leadingAction() const;

signals:
    void searchRequested(const QString &text);

private:
    void refreshIcon();

    QAction *m_searchAction = nullptr;
};
