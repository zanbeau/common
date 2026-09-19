#include "messagebox.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QVBoxLayout>

#include "pushbutton.h"
#include "theme.h"
#include "titlebar.h"

namespace {

// 32x32 的圆形图标 + 字形(i / ! / ✕ / ?),按角色取色
QPixmap makeIconPixmap(MessageBox::Icon icon)
{
    if(icon == MessageBox::Icon::None)
    {
        return {};
    }

    Theme::Role role = Theme::Role::Primary;
    QChar glyph = QLatin1Char('i');
    switch(icon)
    {
    case MessageBox::Icon::Information:
        role = Theme::Role::Primary;
        glyph = QLatin1Char('i');
        break;
    case MessageBox::Icon::Warning:
        role = Theme::Role::Warning;
        glyph = QLatin1Char('!');
        break;
    case MessageBox::Icon::Critical:
        role = Theme::Role::Danger;
        glyph = QLatin1Char('x');
        break;
    case MessageBox::Icon::Question:
        role = Theme::Role::Primary;
        glyph = QLatin1Char('?');
        break;
    default:
        break;
    }

    const QColor color = Theme::instance()->color(role);
    QPixmap pixmap(64, 64);
    pixmap.setDevicePixelRatio(2.0);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(2, 2, 28, 28);
    painter.setPen(Theme::instance()->color(Theme::Role::TextOnPrimary));
    QFont font = painter.font();
    font.setPixelSize(16);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(QRect(2, 2, 28, 28), Qt::AlignCenter, QString(glyph));
    return pixmap;
}

} // namespace

MessageBox::MessageBox(const QString &title, const QString &text, Icon icon, QWidget *parent)
    : FramelessDialog(parent)
{
    setMinimumWidth(340);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    TitleBar *bar = new TitleBar(this, title);
    root->addWidget(bar);

    // 内容区:图标 + 文字
    QWidget *content = new QWidget;
    content->setStyleSheet(QStringLiteral("background:%1;")
                               .arg(Theme::instance()->color(Theme::Role::Background).name()));
    QHBoxLayout *contentRow = new QHBoxLayout(content);
    contentRow->setContentsMargins(20, 20, 20, 16);
    contentRow->setSpacing(12);

    const QPixmap iconPixmap = makeIconPixmap(icon);
    if(!iconPixmap.isNull())
    {
        QLabel *iconLabel = new QLabel;
        iconLabel->setPixmap(iconPixmap);
        iconLabel->setFixedSize(32, 32);
        contentRow->addWidget(iconLabel, 0, Qt::AlignTop);
    }

    m_textLabel = new QLabel(text);
    m_textLabel->setWordWrap(true);
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    QFont textFont = m_textLabel->font();
    textFont.setPixelSize(Theme::instance()->fontPx(Tokens::FontSize::BodyStrong));
    m_textLabel->setFont(textFont);
    contentRow->addWidget(m_textLabel, 1);

    root->addWidget(content, 1);

    // 按钮区:默认单个"确定"(主按钮);确认类场景再补"取消"
    QWidget *buttonRow = new QWidget;
    buttonRow->setStyleSheet(QStringLiteral("background:%1;")
                                 .arg(Theme::instance()->color(Theme::Role::Background).name()));
    QHBoxLayout *buttons = new QHBoxLayout(buttonRow);
    buttons->setContentsMargins(20, 0, 20, 20);
    buttons->setSpacing(8);
    buttons->addStretch();

    auto *okButton = new PushButton(QStringLiteral("确定"), PushButton::Type::Primary);
    okButton->setObjectName(QStringLiteral("okButton"));
    connect(okButton, &PushButton::clicked, this, &MessageBox::accept);
    buttons->addWidget(okButton);

    root->addWidget(buttonRow);
}

void MessageBox::setText(const QString &text)
{
    m_textLabel->setText(text);
}

QString MessageBox::text() const
{
    return m_textLabel->text();
}

void MessageBox::information(QWidget *parent, const QString &title, const QString &text)
{
    MessageBox box(title, text, Icon::Information, parent);
    box.exec();
}

void MessageBox::warning(QWidget *parent, const QString &title, const QString &text)
{
    MessageBox box(title, text, Icon::Warning, parent);
    box.exec();
}

void MessageBox::critical(QWidget *parent, const QString &title, const QString &text)
{
    MessageBox box(title, text, Icon::Critical, parent);
    box.exec();
}

bool MessageBox::question(QWidget *parent, const QString &title, const QString &text)
{
    MessageBox box(title, text, Icon::Question, parent);

    // "取消"插到"确定"左边;Esc / 标题栏关闭由 QDialog 默认走 rejected
    auto *cancelButton = new PushButton(QStringLiteral("取消"));
    connect(cancelButton, &PushButton::clicked, &box, &QDialog::reject);
    for(PushButton *button : box.findChildren<PushButton *>())
    {
        if(button->objectName() == QLatin1String("okButton"))
        {
            QHBoxLayout *row = static_cast<QHBoxLayout *>(button->parentWidget()->layout());
            row->insertWidget(row->indexOf(button), cancelButton);
            break;
        }
    }

    return box.exec() == QDialog::Accepted;
}
