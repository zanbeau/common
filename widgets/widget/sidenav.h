#pragma once

#include <QVector>
#include <QWidget>

// 侧边导航(Ant Menu 风格):垂直条目导航,选中项淡主色底 + 左侧指示条,
// 悬浮/选中/亮暗全部主题化;点击或上下键切换,发 currentChanged(index)。
// 典型用法:与 QStackedWidget 配对:
//   auto *nav = new SideNav;
//   nav->addItem(QStringLiteral("发现音乐"));
//   connect(nav, &SideNav::currentChanged, stack, &QStackedWidget::setCurrentIndex);
class SideNav : public QWidget
{
    Q_OBJECT
public:
    explicit SideNav(QWidget *parent = nullptr);

    int addItem(const QString &text); // 追加条目,返回其索引
    int count() const;
    int currentIndex() const;
    void setCurrentIndex(int index); // 越界忽略;同值不发信号
    QString itemText(int index) const;

    QSize sizeHint() const override;

signals:
    void currentChanged(int index);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    int indexAt(const QPoint &pos) const;
    void setCurrent(int index);

    QVector<QString> m_items;
    int m_current = -1;
    int m_hover = -1;
    int m_pressed = -1;
};
