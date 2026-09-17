// 控件浏览器入口:新增示例页在下面 addPage 一行接入
#include <QApplication>

#include "logger.h"
#include "singleinstance.h"

#include "gallerywindow.h"
#include "pages/clickedlabelpage.h"
#include "pages/framelesspage.h"
#include "pages/toastpage.h"

int main(int argc, char *argv[])
{
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
