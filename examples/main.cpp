// 控件浏览器入口:新增示例页在下面 addPage 一行接入
#include <QApplication>

#include "logger.h"
#include "singleinstance.h"
#include "theme.h"

#include "gallerywindow.h"
#include "pages/animationstackedpage.h"
#include "pages/clickedlabelpage.h"
#include "pages/clickedsliderpage.h"
#include "pages/framelesspage.h"
#include "pages/marqueepage.h"
#include "pages/toastpage.h"
#include "pages/waitspinnerpage.h"
#include "pages/themepage.h"
#include "pages/pushbuttonpage.h"
#include "pages/toggleswitchpage.h"
#include "pages/searchinputpage.h"
#include "pages/tipsliderpage.h"
#include "pages/transitionlabelpage.h"
#include "pages/rotatelabelpage.h"
#include "pages/framelessdialogpage.h"
#include "pages/titlebarpage.h"
#include "pages/messageboxpage.h"
#include "pages/notifywindowpage.h"
#include "pages/splashscreenpage.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5 需在 QApplication 构造前显式开启,Qt6 默认已启用
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    // 应用主题:基础件整体换肤;此后 setMode 即时切换亮暗
    Theme::instance()->apply();

    // 单实例演示:第二个实例把消息转发给第一个后直接退出
    SingleInstance guard(QStringLiteral("canfan-widgets-gallery"));
    if(!guard.isPrimary())
    {
        guard.sendMessage(QStringLiteral("activate"));
        return 0;
    }

    GalleryWindow window;
    window.addPage(QStringLiteral("主题与令牌"), new ThemePage);
    window.addPage(QStringLiteral("按钮"), new PushButtonPage);
    window.addPage(QStringLiteral("开关"), new ToggleSwitchPage);
    window.addPage(QStringLiteral("搜索输入"), new SearchInputPage);
    window.addPage(QStringLiteral("悬停提示滑条"), new TipSliderPage);
    window.addPage(QStringLiteral("封面渐变"), new TransitionLabelPage);
    window.addPage(QStringLiteral("旋转封面"), new RotateLabelPage);
    window.addPage(QStringLiteral("桌面通知"), new NotifyWindowPage);
    window.addPage(QStringLiteral("启动画面"), new SplashScreenPage);
    window.addPage(QStringLiteral("无边框窗口"), new FramelessPage);
    window.addPage(QStringLiteral("无边框对话框"), new FramelessDialogPage);
    window.addPage(QStringLiteral("标题栏"), new TitleBarPage);
    window.addPage(QStringLiteral("消息框"), new MessageBoxPage);
    window.addPage(QStringLiteral("可点击标签"), new ClickedLabelPage);
    window.addPage(QStringLiteral("滚动文本"), new MarqueePage);
    window.addPage(QStringLiteral("点击跳转滑条"), new ClickedSliderPage);
    window.addPage(QStringLiteral("等待转圈"), new WaitSpinnerPage);
    window.addPage(QStringLiteral("页面切换动画"), new AnimationStackedPage);
    window.addPage(QStringLiteral("Toast 提示"), new ToastPage);
    QObject::connect(&guard, &SingleInstance::messageReceived, &window, [&window](const QString &message) {
        Log::info() << "收到副实例消息:" << message;
        window.showNormal();
        window.raise();
        window.activateWindow();
    });

    Log::info() << "gallery started";
    window.show();

    const int code = app.exec();
    Log::info() << "gallery exited, code =" << code;
    return code;
}
