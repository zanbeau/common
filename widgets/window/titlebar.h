#pragma once

#include <QWidget>

class QLabel;

// 无边框窗口的标题栏:标题(左)+ 最小化/最大化/关闭(右),主题化绘制。
// 标题栏通过 FramelessHandler::watch() 纳入窗口的拖动/拉伸/双击最大化体系
// (面板事件按窗口坐标处理),按钮自身响应点击、不受拖动影响。
// 用法(宿主须为 FramelessWidget/FramelessDialog 或挂了 handler 的窗口):
//   auto *bar = new TitleBar(this, QStringLiteral("标题"));
//   layout->addWidget(bar);   // 放在窗口顶部
class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(QWidget *window, const QString &title = {});

    void setTitle(const QString &title);
    QString title() const;

    void setClosable(bool closable); // 关闭按钮显隐
    bool isClosable() const;

    QSize sizeHint() const override;

signals:
    void closeRequested(); // 关闭按钮点击(默认行为:window->close())

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QWidget *m_window;
    QLabel *m_title = nullptr;
    QWidget *m_minButton = nullptr;
    QWidget *m_maxButton = nullptr;
    QWidget *m_closeButton = nullptr;
};
