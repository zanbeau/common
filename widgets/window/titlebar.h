#pragma once

#include <QWidget>

class QLabel;

// 无边框窗口的标题栏:标题(左)+ 最小化/最大化/关闭(右),主题化绘制。
// 自身与标题对鼠标透明(按钮除外),按下/双击事件穿透到宿主窗口,
// 由 FramelessHandler 完成拖动与双击最大化。
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
