#include "framelessdialogpage.h"

#include <QLabel>
#include <QVBoxLayout>

#include "framelessdialog.h"
#include "pushbutton.h"

FramelessDialogPage::FramelessDialogPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "FramelessDialog:无边框对话框基类,与 FramelessWidget 共用同一套\n"
        "拖动/八方向拉伸/双击最大化行为(FramelessHandler)。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PushButton *open = new PushButton(QStringLiteral("打开对话框"), PushButton::Type::Primary);
    connect(open, &PushButton::clicked, this, [this]() {
        FramelessDialog *dialog = new FramelessDialog(this);
        dialog->setWindowTitle(QStringLiteral("无边框对话框"));
        dialog->resize(360, 220);

        QLabel *hint = new QLabel(QStringLiteral(
            "自带阴影与圆角;按住面板拖动窗口;\n"
            "靠近边缘出现拉伸光标;双击面板最大化/还原。"));
        hint->setAlignment(Qt::AlignCenter);

        PushButton *close = new PushButton(QStringLiteral("关闭"), PushButton::Type::Primary);
        connect(close, &PushButton::clicked, dialog, &QDialog::accept);

        QVBoxLayout *dialogLayout = dialog->contentLayout();
        dialogLayout->setContentsMargins(24, 24, 24, 24);
        dialogLayout->addWidget(hint);
        dialogLayout->addStretch();
        dialogLayout->addWidget(close, 0, Qt::AlignHCenter);

        dialog->setAttribute(Qt::WA_DeleteOnClose);
        dialog->show();
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addStretch();
    layout->addWidget(open, 0, Qt::AlignHCenter);
    layout->addStretch();
}
