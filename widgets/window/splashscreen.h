#pragma once

#include <QWidget>

class QPropertyAnimation;
class QShowEvent;

// 启动画面:无边框置顶的启动闪屏(logo 居中 + 底部状态文字),
// 主窗就绪后调用 finish() 淡出并自毁(堆分配,WA_DeleteOnClose)。
// 典型用法:
//   auto *splash = new SplashScreen(QPixmap(":/logo.png"));
//   splash->showMessage("正在加载…");
//   splash->show();
//   ... 初始化 ...
//   splash->finish(&window);
class SplashScreen : public QWidget
{
    Q_OBJECT
public:
    explicit SplashScreen(const QPixmap &pixmap = {}, QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap); // 居中展示的 logo
    QPixmap pixmap() const;
    void showMessage(const QString &message); // 底部状态文字
    QString message() const;

    // 主窗显示后调用:淡出并销毁自己;mainWindow 非空时先把它拉到前台
    void finish(QWidget *mainWindow = nullptr);

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_pixmap;
    QString m_message;
    QPropertyAnimation *m_fade = nullptr;
};
