#include <QtTest>

#include <QMouseEvent>

#include "clickedlabel.h"
#include "framelesswidget.h"
#include "toastlabel.h"

class TstWidgets : public QObject
{
    Q_OBJECT
private slots:
    void clickedLabel();
    void toastLabel();
    void framelessFlags();
    void framelessMove();
    void framelessResize();
    void framelessDoubleClick();
};

void TstWidgets::clickedLabel()
{
    ClickedLabel label(QStringLiteral("text"));
    QSignalSpy spy(&label, &ClickedLabel::clicked);

    QTest::mouseClick(&label, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);

    // 右键不触发
    QTest::mouseClick(&label, Qt::RightButton);
    QCOMPARE(spy.count(), 1);

    // 按下后移出标签再释放,不触发
    QTest::mousePress(&label, Qt::LeftButton);
    QMouseEvent outside(QEvent::MouseButtonRelease, QPointF(-10, -10),
                        QPointF(-10, -10), Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&label, &outside);
    QCOMPARE(spy.count(), 1);
}

void TstWidgets::toastLabel()
{
    QWidget host;
    host.resize(400, 300);
    host.show();

    ToastLabel toast(QStringLiteral("hello"), &host);
    QVERIFY(!toast.isVisible());

    toast.popup(100, 50);
    QVERIFY(toast.isVisible());
    // 居中于父窗口
    QCOMPARE(toast.x(), (host.width() - toast.width()) / 2);

    QTRY_VERIFY_WITH_TIMEOUT(!toast.isVisible(), 3000);
}

void TstWidgets::framelessFlags()
{
    FramelessWidget w;
    QVERIFY(w.windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(w.resizeMargin(), 5);

    w.setResizeMargin(8);
    QCOMPARE(w.resizeMargin(), 8);
}

void TstWidgets::framelessMove()
{
    FramelessWidget w;
    w.resize(400, 300);
    w.show();
    QTRY_VERIFY(w.isVisible());

    // offscreen 平台上 startSystemMove 返回 false,走手动回退路径
    const QPoint oldPos = w.pos();
    const QPoint local(200, 150);
    const QPoint global = w.mapToGlobal(local);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(local), QPointF(global),
                      Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &press);

    const QPoint moved = global + QPoint(50, 30);
    QMouseEvent move(QEvent::MouseMove, QPointF(local + QPoint(50, 30)), QPointF(moved),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &move);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(local + QPoint(50, 30)), QPointF(moved),
                        Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&w, &release);

    QCOMPARE(w.pos(), oldPos + QPoint(50, 30));
}

void TstWidgets::framelessResize()
{
    FramelessWidget w;
    w.resize(400, 300);
    w.show();
    QTRY_VERIFY(w.isVisible());

    // 命中左上角 → 向左上拖动,窗口变大
    const QPoint local(1, 1);
    const QPoint global = w.mapToGlobal(local);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(local), QPointF(global),
                      Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &press);

    const QPoint dragged = global + QPoint(-40, -40);
    QMouseEvent move(QEvent::MouseMove, QPointF(local + QPoint(-40, -40)), QPointF(dragged),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &move);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(local + QPoint(-40, -40)), QPointF(dragged),
                        Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&w, &release);

    QCOMPARE(w.width(), 440);
    QCOMPARE(w.height(), 340);
}

void TstWidgets::framelessDoubleClick()
{
    FramelessWidget w;
    w.resize(400, 300);
    w.show();
    QTRY_VERIFY(w.isVisible());

    const QPointF local(200, 150);
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, local, local + QPointF(w.x(), w.y()),
                         Qt::LeftButton, Qt::LeftButton, {});

    QApplication::sendEvent(&w, &dblClick);
    QTRY_VERIFY(w.isMaximized());

    QApplication::sendEvent(&w, &dblClick);
    QTRY_VERIFY(!w.isMaximized());
}

QTEST_MAIN(TstWidgets)
#include "tst_widgets.moc"
