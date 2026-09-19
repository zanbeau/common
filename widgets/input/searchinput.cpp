#include "searchinput.h"

#include <QAction>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

#include "theme.h"

namespace {

// 用主题色画一个 16x16 的放大镜(2x 采样保证清晰)
QIcon makeSearchIcon(const QColor &color)
{
    QPixmap pixmap(32, 32);
    pixmap.setDevicePixelRatio(2.0);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen(color, 1.6);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(QPointF(7.0, 7.0), 4.2, 4.2);
    painter.drawLine(QPointF(10.2, 10.2), QPointF(13.4, 13.4));
    return QIcon(pixmap);
}

}

SearchInput::SearchInput(const QString &placeholder, QWidget *parent)
    : QLineEdit(parent)
{
    setPlaceholderText(placeholder.isEmpty() ? QStringLiteral("搜索") : placeholder);
    setClearButtonEnabled(true);

    m_searchAction = addAction(makeSearchIcon(
                        Theme::instance()->color(Theme::Role::TextSecondary)),
                        QLineEdit::LeadingPosition);

    connect(this, &SearchInput::returnPressed, this, [this]() {
        emit searchRequested(text());
    });
    connect(Theme::instance(), &Theme::modeChanged, this, &SearchInput::refreshIcon);
}

QAction *SearchInput::leadingAction() const
{
    return m_searchAction;
}

void SearchInput::refreshIcon()
{
    m_searchAction->setIcon(makeSearchIcon(
        Theme::instance()->color(Theme::Role::TextSecondary)));
}
