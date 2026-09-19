#include "splashscreenpage.h"

#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QTimer>
#include <QVBoxLayout>

#include "pushbutton.h"
#include "splashscreen.h"

namespace {

QPixmap makeLogo()
{
    QPixmap pixmap(128, 128);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(0x16, 0x5D, 0xFF));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(pixmap.rect(), 28, 28);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(64);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QStringLiteral("C"));
    return pixmap;
}

} // namespace

SplashScreenPage::SplashScreenPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "SplashScreen:启动画面(logo 居中 + 状态文字),\n"
        "主窗就绪后 finish() 淡出自毁。演示:显示 1.5 秒后收场。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *show = new PushButton(QStringLiteral("播放启动流程"), PushButton::Type::Primary);
    connect(show, &PushButton::clicked, this, [this]() {
        auto *splash = new SplashScreen(makeLogo());
        splash->showMessage(QStringLiteral("正在加载模块…"));
        splash->show();
        QTimer::singleShot(1500, this, [splash, this]() {
            if(splash)
            {
                splash->showMessage(QStringLiteral("即将就绪"));
                splash->finish(window());
            }
        });
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(show, 0, Qt::AlignHCenter);
    layout->addStretch();
}
