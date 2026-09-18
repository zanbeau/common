#include <QtTest>

#include <QEvent>
#include <QLabel>
#include <QMouseEvent>

#include "animationstackedwidget.h"
#include "clickedlabel.h"
#include "clickedslider.h"
#include "framelesswidget.h"
#include "marqueelabel.h"
#include "toastlabel.h"
#include "waitspinner.h"

class TstWidgets : public QObject
{
    Q_OBJECT
private slots:
    void clickedLabel();
    void toastLabel();
    void clickedSlider();
    void marqueeLabel();
    void waitSpinner();
    void animationStackedWidget();
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

void TstWidgets::clickedSlider()
{
    ClickedSlider slider(Qt::Horizontal);
    slider.setRange(0, 100);
    slider.resize(200, 30);
    slider.show();
    QTRY_VERIFY(slider.isVisible());

    // 点击右端附近,手柄应跳到高位并发出 clicked()
    QSignalSpy spy(&slider, &ClickedSlider::clicked);
    QTest::mouseClick(&slider, Qt::LeftButton, {}, QPoint(180, 15));
    QVERIFY(slider.value() > 70);
    QCOMPARE(spy.count(), 1);

    // 点击后按住继续拖动仍生效(再次按下会再发一次 clicked)
    QTest::mousePress(&slider, Qt::LeftButton, {}, QPoint(180, 15));
    QCOMPARE(spy.count(), 2);
    QMouseEvent drag(QEvent::MouseMove, QPointF(190, 15), QPointF(190, 15),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&slider, &drag);
    const int dragged = slider.value();
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(190, 15), QPointF(190, 15),
                        Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&slider, &release);
    QVERIFY(dragged > 80);

    // 右键不处理
    const int before = slider.value();
    const int clicks = spy.count();
    QTest::mouseClick(&slider, Qt::RightButton, {}, QPoint(20, 15));
    QCOMPARE(spy.count(), clicks);
    QCOMPARE(slider.value(), before);
}

void TstWidgets::marqueeLabel()
{
    MarqueeLabel label;
    label.resize(120, 24);

    label.setText(QStringLiteral("short"));
    QVERIFY(!label.isScrolling());

    // 文字远超宽度进入滚动模式,且随时间推进。
    // offscreen 的虚拟光标默认停在 (10,10),把标签挪远避开
    label.setText(QString(100, QLatin1Char('A')));
    QVERIFY(label.isScrolling());
    label.move(400, 300);
    label.show();
    QTRY_VERIFY(label.isVisible());
    QTRY_VERIFY(label.offset() != 0);

    // hover 时暂停滚动(直接发 Enter/Leave;offscreen 不随 setPos 派发)
    const int frozen = label.offset();
    QEvent enter(QEvent::Enter);
    QApplication::sendEvent(&label, &enter);
    QTest::qWait(200);
    QCOMPARE(label.offset(), frozen);

    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(&label, &leave);
    QTRY_VERIFY(label.offset() != frozen);

    // 换回短文字自动回到静止模式
    label.setText(QStringLiteral("short"));
    QVERIFY(!label.isScrolling());
}

void TstWidgets::waitSpinner()
{
    WaitSpinner spinner;
    spinner.setLoopDuration(200);
    spinner.resize(32, 32);
    spinner.show();

    QVERIFY(!spinner.isSpinning());
    spinner.start();
    QVERIFY(spinner.isSpinning());
    QTRY_VERIFY(spinner.angle() > 0);

    spinner.stop();
    QVERIFY(!spinner.isSpinning());

    // 属性回读
    spinner.setLoopDuration(500);
    QCOMPARE(spinner.loopDuration(), 500);
    spinner.setLineWidth(5);
    QCOMPARE(spinner.lineWidth(), 5);
}

void TstWidgets::animationStackedWidget()
{
    AnimationStackedWidget stack;
    QLabel *page0 = new QLabel(QStringLiteral("0"));
    QLabel *page1 = new QLabel(QStringLiteral("1"));
    QLabel *page2 = new QLabel(QStringLiteral("2"));
    stack.addWidget(page0);
    stack.addWidget(page1);
    stack.addWidget(page2);
    stack.resize(300, 200);
    stack.show();
    QTRY_VERIFY(stack.isVisible());

    stack.setDuration(100);
    QCOMPARE(stack.currentIndex(), 0);

    stack.setCurrentIndexAnimated(2);
    QVERIFY(stack.isAnimating());
    QTRY_VERIFY(!stack.isAnimating());
    QCOMPARE(stack.currentIndex(), 2);
    QVERIFY(page2->isVisible());
    QVERIFY(!page0->isVisible());
    QCOMPARE(page0->pos(), QPoint(0, 0));

    // 反向切回
    stack.setCurrentIndexAnimated(0);
    QTRY_VERIFY(!stack.isAnimating());
    QCOMPARE(stack.currentIndex(), 0);
    QVERIFY(page0->isVisible());

    // 动画期间的新请求被忽略(用超长动画验证)
    stack.setDuration(60000);
    stack.setCurrentIndexAnimated(1);
    QVERIFY(stack.isAnimating());
    stack.setCurrentIndexAnimated(2);
    QCOMPARE(stack.currentIndex(), 1);
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

// 等价 QTEST_MAIN,但需在 QApplication 构造前为 Qt5 开启 High-DPI
int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    TstWidgets tc;
    return QTest::qExec(&tc, argc, argv);
}
#include "tst_widgets.moc"
