#include <QtTest>

#include <QEvent>
#include <QImage>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QPointer>
#include <QVBoxLayout>

#include "animationstackedwidget.h"
#include "clickedlabel.h"
#include "clickedslider.h"
#include "coverflow.h"
#include "duration.h"
#include "framelessdialog.h"
#include "framelesshandler.h"
#include "framelesswidget.h"
#include "marqueelabel.h"
#include "messagebox.h"
#include "notifywindow.h"
#include "pushbutton.h"
#include "rotatelabel.h"
#include "searchinput.h"
#include "sidenav.h"
#include "splashscreen.h"
#include "theme.h"
#include "tipslider.h"
#include "toastlabel.h"
#include "titlebar.h"
#include "toggleswitch.h"
#include "transitionlabel.h"
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
    void theme();
    void framelessDialog();
    void notifyWindow();
    void splashScreen();
    void pushButton();
    void toggleSwitch();
    void searchInput();
    void tipSlider();
    void transitionLabel();
    void rotateLabel();
    void titleBar();
    void messageBox();
    void sideNav();
    void coverFlow();
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

    // 主题化样式:反色深底 + 令牌圆角,切暗色后颜色随之变化
    const QString lightSheet = toast.styleSheet();
    QVERIFY(lightSheet.contains(
        Theme::instance()->color(Theme::Role::InverseSurface).name()));
    Theme::instance()->setMode(Theme::Mode::Dark);
    QVERIFY(toast.styleSheet() != lightSheet);
    Theme::instance()->setMode(Theme::Mode::Light);

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

    // 空量程时按下必须被接收:QSlider 默认 ignore 会冒泡到
    // 无边框父窗口,被当成"空白区按下"而拖动整个窗口
    ClickedSlider empty(Qt::Horizontal);
    QMouseEvent emptyPress(QEvent::MouseButtonPress, QPointF(10, 5), QPointF(10, 5),
                           Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&empty, &emptyPress);
    QVERIFY(emptyPress.isAccepted());
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

    // 默认色取主题主色;设过自定义色后不再跟随主题
    QCOMPARE(spinner.color(), Theme::instance()->color(Theme::Role::Primary));
    spinner.setColor(QColor(0, 128, 0));
    Theme::instance()->setMode(Theme::Mode::Dark);
    QCOMPARE(spinner.color(), QColor(0, 128, 0));
    Theme::instance()->setMode(Theme::Mode::Light);

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

    // 自 v0.10.0 起,拖动只认 watch 面板(标题栏);内容区按下不拖动窗口。
    // 这里用一个 watch 面板模拟标题栏
    QWidget dragPanel(&w);
    dragPanel.setGeometry(0, 0, 400, 32);
    dragPanel.show();
    FramelessHandler drag(&w, &w, false); // 只 watch,不过滤窗口本体
    drag.watch(&dragPanel);

    // offscreen 平台上 startSystemMove 返回 false,走手动回退路径
    const QPoint oldPos = w.pos();
    const QPoint panelLocal(200, 15);
    const QPoint global = dragPanel.mapToGlobal(panelLocal);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(panelLocal), QPointF(global),
                      Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&dragPanel, &press);

    const QPoint moved = global + QPoint(50, 30);
    QMouseEvent move(QEvent::MouseMove, QPointF(panelLocal + QPoint(50, 30)), QPointF(moved),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&dragPanel, &move);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(panelLocal + QPoint(50, 30)),
                        QPointF(moved), Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&dragPanel, &release);

    QCOMPARE(w.pos(), oldPos + QPoint(50, 30));

    // 内容区(窗口本体)按下拖动:窗口不动(播放栏空白处拖动的回归用例)
    const QPoint contentLocal(200, 150);
    const QPoint contentGlobal = w.mapToGlobal(contentLocal);
    QMouseEvent contentPress(QEvent::MouseButtonPress, QPointF(contentLocal),
                             QPointF(contentGlobal), Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &contentPress);
    QMouseEvent contentMove(QEvent::MouseMove, QPointF(contentLocal + QPoint(60, 40)),
                            QPointF(contentGlobal + QPoint(60, 40)),
                            Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &contentMove);
    QMouseEvent contentRelease(QEvent::MouseButtonRelease,
                               QPointF(contentLocal + QPoint(60, 40)),
                               QPointF(contentGlobal + QPoint(60, 40)),
                               Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&w, &contentRelease);
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

    // 设置了最大尺寸后,手动拉伸不能超过上限
    FramelessWidget capped;
    capped.setMaximumWidth(450);
    capped.setMaximumHeight(350);
    capped.resize(400, 300);
    capped.show();
    QTRY_VERIFY(capped.isVisible());

    const QPoint local2(1, 1);
    const QPoint global2 = capped.mapToGlobal(local2);

    QMouseEvent press2(QEvent::MouseButtonPress, QPointF(local2), QPointF(global2),
                       Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&capped, &press2);

    const QPoint dragged2 = global2 + QPoint(-80, -80);
    QMouseEvent move2(QEvent::MouseMove, QPointF(local2 + QPoint(-80, -80)), QPointF(dragged2),
                      Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&capped, &move2);

    QMouseEvent release2(QEvent::MouseButtonRelease, QPointF(local2 + QPoint(-80, -80)), QPointF(dragged2),
                         Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&capped, &release2);

    // 想拉到 480x380,被钳到 450x350
    QCOMPARE(capped.width(), 450);
    QCOMPARE(capped.height(), 350);
}

void TstWidgets::framelessDoubleClick()
{
    FramelessWidget w;
    w.resize(400, 300);
    w.show();
    QTRY_VERIFY(w.isVisible());

    // 双击最大化只认 watch 面板;用面板模拟标题栏
    QWidget dragPanel(&w);
    dragPanel.setGeometry(0, 0, 400, 32);
    dragPanel.show();
    FramelessHandler drag(&w, &w, false);
    drag.watch(&dragPanel);

    const QPointF panelLocal(200, 15);
    QMouseEvent dblClick(QEvent::MouseButtonDblClick, panelLocal,
                         panelLocal + QPointF(w.x(), w.y()),
                         Qt::LeftButton, Qt::LeftButton, {});

    QApplication::sendEvent(&dragPanel, &dblClick);
    QTRY_VERIFY(w.isMaximized());

    QApplication::sendEvent(&dragPanel, &dblClick);
    QTRY_VERIFY(!w.isMaximized());

    // 内容区双击不再最大化(v0.10.0 语义)
    const QPointF content(200, 150);
    QMouseEvent contentDbl(QEvent::MouseButtonDblClick, content,
                           content + QPointF(w.x(), w.y()),
                           Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&w, &contentDbl);
    QVERIFY(!w.isMaximized());
}

void TstWidgets::theme()
{
    Theme *theme = Theme::instance();
    QCOMPARE(theme->mode(), Theme::Mode::Light);

    QSignalSpy spy(theme, &Theme::modeChanged);
    const QColor lightBg = theme->color(Theme::Role::Background);

    theme->setMode(Theme::Mode::Dark);
    QCOMPARE(spy.count(), 1);
    QVERIFY(theme->color(Theme::Role::Background) != lightBg);

    // 重复设置不触发
    theme->setMode(Theme::Mode::Dark);
    QCOMPARE(spy.count(), 1);

    // 亮暗两套都满足"底色浅、文字深"(或反之)的可读性约束
    const QColor darkText = theme->color(Theme::Role::Text);
    const QColor darkBg = theme->color(Theme::Role::Background);
    QVERIFY(darkText.lightness() > darkBg.lightness());

    // QSS 生成且随模式变化;apply() 后挂到应用上,setMode 即时刷新
    const QString darkSheet = theme->styleSheet();
    QVERIFY(!darkSheet.isEmpty());
    theme->apply();
    QVERIFY(!qApp->styleSheet().isEmpty());
    theme->setMode(Theme::Mode::Light);
    QVERIFY(qApp->styleSheet() != darkSheet);

    // 回归:apply() 必须先调色板后样式表。QSS 下已存在控件只在重 polish 时
    // 快照应用调色板,顺序反了会慢一拍——切回亮色后 QLabel 白底白字"消失"
    QLabel follower(QStringLiteral("主题切换跟随"));
    follower.ensurePolished();
    theme->setMode(Theme::Mode::Dark);
    QCOMPARE(follower.palette().color(QPalette::WindowText),
             theme->color(Theme::Role::Text));
    theme->setMode(Theme::Mode::Light);
    QCOMPARE(follower.palette().color(QPalette::WindowText),
             theme->color(Theme::Role::Text));

    // 尺寸/字号令牌透出
    QCOMPARE(theme->radius(Tokens::Radius::MD), 6);
    QCOMPARE(theme->spacing(Tokens::Spacing::LG), 16);
    QCOMPARE(theme->fontPx(Tokens::FontSize::Body), 13);
}

void TstWidgets::framelessDialog()
{
    FramelessDialog dialog;
    QVERIFY(dialog.windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(dialog.resizeMargin(), 5);
    dialog.setResizeMargin(9);
    QCOMPARE(dialog.resizeMargin(), 9);

    // 与 FramelessWidget 同一套拖动行为(面板拖动;offscreen 走手动回退路径)
    dialog.resize(400, 300);
    dialog.show();
    QTRY_VERIFY(dialog.isVisible());

    QWidget dragPanel(&dialog);
    dragPanel.setGeometry(0, 0, 400, 32);
    dragPanel.show();
    FramelessHandler drag(&dialog, &dialog, false);
    drag.watch(&dragPanel);

    const QPoint oldPos = dialog.pos();
    const QPoint local(200, 15);
    const QPoint global = dragPanel.mapToGlobal(local);

    QMouseEvent press(QEvent::MouseButtonPress, QPointF(local), QPointF(global),
                      Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&dragPanel, &press);

    const QPoint moved = global + QPoint(40, 20);
    QMouseEvent move(QEvent::MouseMove, QPointF(local + QPoint(40, 20)), QPointF(moved),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&dragPanel, &move);

    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(local + QPoint(40, 20)),
                        QPointF(moved), Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&dragPanel, &release);

    QCOMPARE(dialog.pos(), oldPos + QPoint(40, 20));
}

void TstWidgets::notifyWindow()
{
    // 到点自动淡出销毁
    QPointer<NotifyWindow> expired =
        NotifyWindow::showMessage(QStringLiteral("标题"), QStringLiteral("内容"), 400);
    QVERIFY(expired->isVisible());
    QTRY_VERIFY_WITH_TIMEOUT(expired.isNull(), 5000);

    // 点击触发 clicked 并立即关闭
    QPointer<NotifyWindow> clicked =
        NotifyWindow::showMessage(QStringLiteral("标题"), QStringLiteral("内容"), 60000);
    QTRY_VERIFY(clicked->isVisible());
    QSignalSpy spy(clicked.data(), &NotifyWindow::clicked);
    QTest::mouseClick(clicked.data(), Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QTRY_VERIFY_WITH_TIMEOUT(clicked.isNull(), 3000);
}

void TstWidgets::splashScreen()
{
    QPixmap logo(64, 64);
    logo.fill(Qt::blue);

    QPointer<SplashScreen> splash = new SplashScreen(logo);
    splash->show();
    QTRY_VERIFY(splash->isVisible());

    splash->showMessage(QStringLiteral("正在加载"));
    QCOMPARE(splash->message(), QStringLiteral("正在加载"));
    QCOMPARE(splash->pixmap().size(), QSize(64, 64));

    // finish() 淡出后自毁
    splash->finish();
    QTRY_VERIFY_WITH_TIMEOUT(splash.isNull(), 5000);
}

void TstWidgets::pushButton()
{
    PushButton button(QStringLiteral("确定"), PushButton::Type::Primary);
    QCOMPARE(button.type(), PushButton::Type::Primary);

    // QPushButton 惯用形态 (text, parent) 的便捷构造
    QWidget host;
    PushButton convenience(QStringLiteral("便捷"), &host);
    QCOMPARE(convenience.type(), PushButton::Type::Default);
    QCOMPARE(convenience.parentWidget(), &host);

    QSignalSpy spy(&button, &PushButton::clicked);
    button.setType(PushButton::Type::Danger);
    QCOMPARE(button.type(), PushButton::Type::Danger);
    QTest::mouseClick(&button, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);

    // 四种类型(含禁用态)都能完成一次自绘
    for(int type = 0; type <= static_cast<int>(PushButton::Type::Text); ++type)
    {
        PushButton sample(QStringLiteral("按钮"), static_cast<PushButton::Type>(type));
        sample.setEnabled(false);
        QVERIFY(!sample.grab().isNull());
    }
    QVERIFY(!button.grab().isNull());
}

void TstWidgets::toggleSwitch()
{
    ToggleSwitch toggle;
    toggle.show();
    QVERIFY(!toggle.isChecked());

    QSignalSpy toggledSpy(&toggle, &ToggleSwitch::toggled);
    QTest::mouseClick(&toggle, Qt::LeftButton);
    QVERIFY(toggle.isChecked());
    QCOMPARE(toggledSpy.count(), 1);
    QCOMPARE(toggledSpy.takeFirst().at(0).toBool(), true); // takeFirst 后计数归零
    // 滑块动效推进到开位
    QTRY_COMPARE(toggle.knobPos(), 1.0);

    // setChecked 同值不发信号
    toggle.setChecked(true);
    QCOMPARE(toggledSpy.count(), 0);
    toggle.setChecked(false);
    QCOMPARE(toggledSpy.count(), 1);

    // 空格键切换
    QTest::keyClick(&toggle, Qt::Key_Space);
    QVERIFY(toggle.isChecked());
    QCOMPARE(toggledSpy.count(), 2);
    QVERIFY(!toggle.grab().isNull());

    // 焦点圈只在键盘导航(Tab)获得焦点时绘制,鼠标点击的焦点不改变外观
    QTRY_COMPARE(toggle.knobPos(), 1.0);
    const QImage baseImage = toggle.grab().toImage();
    toggle.setFocus(Qt::MouseFocusReason);
    QVERIFY(toggle.hasFocus());
    QCOMPARE(toggle.grab().toImage(), baseImage);

    toggle.clearFocus();
    toggle.setFocus(Qt::TabFocusReason);
    const QImage tabFocusImage = toggle.grab().toImage();
    QVERIFY(tabFocusImage != baseImage);  // Tab 焦点画出焦点圈

    toggle.clearFocus();
    QVERIFY(!toggle.hasFocus());
    QCOMPARE(toggle.grab().toImage(), baseImage);  // 失焦后焦点圈消失
}

void TstWidgets::searchInput()
{
    SearchInput input(QStringLiteral("搜索歌曲"));
    QCOMPARE(input.placeholderText(), QStringLiteral("搜索歌曲"));
    QVERIFY(input.actions().contains(input.leadingAction()));

    input.setText(QStringLiteral("hello"));
    QSignalSpy spy(&input, &SearchInput::searchRequested);
    QTest::keyClick(&input, Qt::Key_Return);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), QStringLiteral("hello"));
}

void TstWidgets::tipSlider()
{
    TipSlider slider(Qt::Horizontal);
    slider.setRange(0, 60000);
    slider.resize(200, 30);
    slider.show();
    QTRY_VERIFY(slider.isVisible());

    // 悬停到中点附近:气泡显示,文案是中点值的 mm:ss
    QMouseEvent move(QEvent::MouseMove, QPointF(100, 15), QPointF(100, 15),
                     Qt::NoButton, Qt::NoButton, {});
    QApplication::sendEvent(&slider, &move);
    QVERIFY(slider.isTipVisible());
    QCOMPARE(slider.tipText(), QStringLiteral("00:30"));

    // 自定义 formatter 生效
    slider.setFormatter([](int value) { return QString::number(value / 1000) + QStringLiteral("s"); });
    QApplication::sendEvent(&slider, &move);
    QCOMPARE(slider.tipText(), QStringLiteral("30s"));

    // 移出后气泡收起
    QEvent leave(QEvent::Leave);
    QApplication::sendEvent(&slider, &leave);
    QVERIFY(!slider.isTipVisible());
}

void TstWidgets::transitionLabel()
{
    QPixmap first(64, 64), second(64, 64);
    first.fill(Qt::red);
    second.fill(Qt::green);

    TransitionLabel label;
    label.resize(128, 128);
    label.show();

    // 首次设置直接显示,不动效
    label.setPixmap(first);
    QVERIFY(!label.isAnimating());

    // 第二次设置触发交叉渐变,完成后停在新的图上
    label.setPixmap(second);
    QVERIFY(label.isAnimating());
    QTRY_VERIFY(!label.isAnimating());
    QCOMPARE(label.pixmap().toImage(), second.toImage());

    label.setDuration(50);
    QCOMPARE(label.duration(), 50);
    label.setPixmap(first);
    QVERIFY(label.isAnimating());
    QTRY_VERIFY_WITH_TIMEOUT(!label.isAnimating(), 2000);
    QVERIFY(!label.grab().isNull());
}

void TstWidgets::rotateLabel()
{
    QPixmap cover(100, 100);
    cover.fill(Qt::darkBlue);

    RotateLabel label;
    label.setPixmap(cover);
    label.resize(160, 160);
    label.show();

    QVERIFY(!label.isRunning());
    label.setRunning(true);
    QVERIFY(label.isRunning());
    QTRY_VERIFY(label.angle() > 0);

    label.setLoopDuration(2000);
    QCOMPARE(label.loopDuration(), 2000);
    label.setCircular(true);
    QVERIFY(label.isCircular());
    QVERIFY(!label.grab().isNull());

    label.setRunning(false);
    QVERIFY(!label.isRunning());
}

void TstWidgets::titleBar()
{
    FramelessWidget window;
    window.resize(400, 300);
    QVBoxLayout *windowLayout = new QVBoxLayout(&window);
    windowLayout->setContentsMargins(0, 0, 0, 0);
    windowLayout->setSpacing(0);
    TitleBar bar(&window, QStringLiteral("测试窗口"));
    windowLayout->addWidget(&bar);
    window.show();
    QTRY_VERIFY(window.isVisible());

    // 标题读写 / 关闭按钮显隐
    QCOMPARE(bar.title(), QStringLiteral("测试窗口"));
    bar.setTitle(QStringLiteral("新标题"));
    QCOMPARE(bar.title(), QStringLiteral("新标题"));
    QVERIFY(bar.isClosable());
    bar.setClosable(false);
    QVERIFY(!bar.isClosable());
    bar.setClosable(true);

    // 最大化/还原按钮驱动窗口状态(事件过滤器切换图形)
    QWidget *maxButton = bar.findChild<QWidget *>(QStringLiteral("maxButton"));
    QVERIFY(maxButton);
    QTest::mouseClick(maxButton, Qt::LeftButton);
    QTRY_VERIFY(window.isMaximized());
    QTest::mouseClick(maxButton, Qt::LeftButton);
    QTRY_VERIFY(!window.isMaximized());

    // 关闭按钮:先发 closeRequested 再关窗
    QSignalSpy spy(&bar, &TitleBar::closeRequested);
    QWidget *closeButton = bar.findChild<QWidget *>(QStringLiteral("closeButton"));
    QVERIFY(closeButton);
    QTest::mouseClick(closeButton, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);
    QTRY_VERIFY(!window.isVisible());
    QVERIFY(!bar.grab().isNull()); // 关闭后仍可离屏自绘

    // 标题栏空白处按下拖动 → 窗口移动(watch 面板路径,不冒泡)
    window.show();
    QTRY_VERIFY(window.isVisible());
    const QPoint oldPos = window.pos();
    const QPoint barLocal(100, 15);
    QMouseEvent barPress(QEvent::MouseButtonPress, QPointF(barLocal),
                         QPointF(bar.mapToGlobal(barLocal)),
                         Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&bar, &barPress);
    const QPoint barMoved = bar.mapToGlobal(barLocal) + QPoint(40, 20);
    QMouseEvent barMove(QEvent::MouseMove, QPointF(barLocal + QPoint(40, 20)),
                        QPointF(barMoved), Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&bar, &barMove);
    QMouseEvent barRelease(QEvent::MouseButtonRelease, QPointF(barLocal + QPoint(40, 20)),
                           QPointF(barMoved), Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&bar, &barRelease);
    QCOMPARE(window.pos(), oldPos + QPoint(40, 20));
}

void TstWidgets::messageBox()
{
    MessageBox box(QStringLiteral("提示"), QStringLiteral("已保存到本地"),
                   MessageBox::Icon::Information);
    QVERIFY(box.windowFlags() & Qt::FramelessWindowHint);
    QCOMPARE(box.text(), QStringLiteral("已保存到本地"));
    box.setText(QStringLiteral("内容已更新"));
    QCOMPARE(box.text(), QStringLiteral("内容已更新"));

    box.show();
    QTRY_VERIFY(box.isVisible());

    // 点"确定"走 accepted
    QSignalSpy spy(&box, &MessageBox::accepted);
    PushButton *ok = nullptr;
    for(PushButton *button : box.findChildren<PushButton *>())
    {
        if(button->objectName() == QLatin1String("okButton"))
        {
            ok = button;
            break;
        }
    }
    QVERIFY(ok);
    QTest::mouseClick(ok, Qt::LeftButton);
    QCOMPARE(spy.count(), 1);

    // 四种图标都能构造并完成一次自绘
    for(int icon = 0; icon <= static_cast<int>(MessageBox::Icon::Question); ++icon)
    {
        MessageBox sample(QStringLiteral("t"), QStringLiteral("b"),
                          static_cast<MessageBox::Icon>(icon));
        QVERIFY(!sample.grab().isNull());
    }
}

void TstWidgets::sideNav()
{
    SideNav nav;
    nav.resize(200, 200);
    nav.show();

    // 首个条目自动选中,追加返回递增索引
    QCOMPARE(nav.addItem(QStringLiteral("发现音乐")), 0);
    QCOMPARE(nav.addItem(QStringLiteral("我的歌单")), 1);
    QCOMPARE(nav.addItem(QStringLiteral("设置")), 2);
    QCOMPARE(nav.count(), 3);
    QCOMPARE(nav.currentIndex(), 0);
    QCOMPARE(nav.itemText(1), QStringLiteral("我的歌单"));

    // 点击第 3 项:发 currentChanged(2)
    QSignalSpy spy(&nav, &SideNav::currentChanged);
    QTest::mouseClick(&nav, Qt::LeftButton, {}, QPoint(100, 2 * 36 + 20));
    QCOMPARE(nav.currentIndex(), 2);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toInt(), 2);

    // setCurrentIndex 越界忽略、同值静默
    nav.setCurrentIndex(5);
    QCOMPARE(nav.currentIndex(), 2);
    nav.setCurrentIndex(2);
    QCOMPARE(spy.count(), 0);

    // 上下键导航
    QTest::keyClick(&nav, Qt::Key_Up);
    QCOMPARE(nav.currentIndex(), 1);
    QCOMPARE(spy.count(), 1);
    QTest::keyClick(&nav, Qt::Key_Up);
    QTest::keyClick(&nav, Qt::Key_Up); // 到顶不再移动
    QCOMPARE(nav.currentIndex(), 0);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!nav.grab().isNull());
}

void TstWidgets::coverFlow()
{
    CoverFlow flow;
    flow.resize(480, 220);
    flow.show();

    QVector<QPixmap> covers;
    for(int i = 0; i < 4; ++i)
    {
        QPixmap cover(120, 120);
        cover.fill(Qt::GlobalColor(Qt::red + i));
        covers.append(cover);
    }
    flow.setCovers(covers);
    QCOMPARE(flow.count(), 4);
    QCOMPARE(flow.currentIndex(), 0);
    QCOMPARE(flow.position(), 0.0);

    // 编程切换:动画滑到目标位并发信号
    QSignalSpy spy(&flow, &CoverFlow::currentChanged);
    flow.setCurrentIndex(3);
    QCOMPARE(spy.count(), 1);
    QTRY_COMPARE(flow.position(), 3.0);

    // 在末张(3)向右拖回一张:释放后吸附到 2 并发信号
    QTest::mousePress(&flow, Qt::LeftButton, {}, QPoint(300, 110));
    QMouseEvent drag(QEvent::MouseMove, QPointF(380, 110), QPointF(380, 110),
                     Qt::LeftButton, Qt::LeftButton, {});
    QApplication::sendEvent(&flow, &drag);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(380, 110), QPointF(380, 110),
                        Qt::LeftButton, Qt::NoButton, {});
    QApplication::sendEvent(&flow, &release);
    QCOMPARE(flow.currentIndex(), 2);
    QCOMPARE(spy.count(), 2);
    QTRY_COMPARE(flow.position(), 2.0);

    // 越界与同值
    flow.setCurrentIndex(9);
    QCOMPARE(flow.currentIndex(), 2);
    flow.setCurrentIndex(2);
    QCOMPARE(spy.count(), 2);
    QVERIFY(!flow.grab().isNull());
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
