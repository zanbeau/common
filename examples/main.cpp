// 控件浏览器入口:新增示例页在下面 addPage 一行接入
#include <QApplication>

#include "logger.h"
#include "singleinstance.h"

#include "gallerywindow.h"
#include "pages/animationstackedpage.h"
#include "pages/clickedlabelpage.h"
#include "pages/clickedsliderpage.h"
#include "pages/framelesspage.h"
#include "pages/marqueepage.h"
#include "pages/toastpage.h"
#include "pages/waitspinnerpage.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Qt5 需在 QApplication 构造前显式开启,Qt6 默认已启用
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    // 单实例演示:第二个实例把消息转发给第一个后直接退出
    SingleInstance guard(QStringLiteral("canfan-widgets-gallery"));
    if(!guard.isPrimary())
    {
        guard.sendMessage(QStringLiteral("activate"));
        return 0;
    }

    GalleryWindow window;
    window.addPage(QStringLiteral("无边框窗口"), new FramelessPage);
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
