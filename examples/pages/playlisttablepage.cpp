#include "playlisttablepage.h"

#include <QLabel>
#include <QPainter>
#include <QRandomGenerator>
#include <QVBoxLayout>

#include "playlisttable.h"
#include "toastlabel.h"

namespace {

// 演示封面:双色渐变 + 序号
QPixmap makeCover(int index)
{
    static const QColor tops[] = {
        QColor(0x16, 0x5D, 0xFF), QColor(0x00, 0xB4, 0x2A), QColor(0xFF, 0x7D, 0x00),
        QColor(0xF5, 0x3F, 0x3F), QColor(0x7B, 0x2F, 0xFF), QColor(0x00, 0xA8, 0xA8)};
    static const QColor bottoms[] = {
        QColor(0x0E, 0x42, 0xD2), QColor(0x00, 0x6A, 0x19), QColor(0xD9, 0x60, 0x00),
        QColor(0xC1, 0x24, 0x24), QColor(0x4E, 0x1D, 0xCC), QColor(0x00, 0x6F, 0x6F)};

    QPixmap pixmap(96, 96);
    QPainter painter(&pixmap);
    QLinearGradient gradient(0, 0, 0, 96);
    gradient.setColorAt(0.0, tops[index % 6]);
    gradient.setColorAt(1.0, bottoms[index % 6]);
    painter.fillRect(pixmap.rect(), gradient);
    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPixelSize(36);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, QString::number(index + 1));
    return pixmap;
}

} // namespace

PlaylistTablePage::PlaylistTablePage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "PlaylistTable:播放列表——序号 / 封面缩略 / 标题+艺术家 / 时长。\n"
        "单击选中,双击播放(正在播放行主色高亮 + 左侧指示条);上下键/PgUp/PgDn/Home/End 导航,回车播放。\n"
        "视口虚拟化绘制,行数再多也只画可见区。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    PlaylistTable *table = new PlaylistTable;
    QVector<PlaylistTable::Track> tracks;
    for(int i = 0; i < 60; ++i)
    {
        PlaylistTable::Track track;
        track.title = QStringLiteral("演示曲目 %1").arg(i + 1, 2, 10, QLatin1Char('0'));
        track.artist = QStringLiteral("艺术家 %1").arg(i % 9 + 1);
        track.duration = 150000 + QRandomGenerator::global()->bounded(180000);
        if(i % 3 != 2)   // 留一部分无封面,演示缩略图列的自适应
        {
            track.cover = makeCover(i);
        }
        tracks.append(track);
    }
    table->setTracks(tracks);

    QLabel *current = new QLabel(QStringLiteral("选中:-"));
    connect(table, &PlaylistTable::currentChanged, this, [current, table](int index) {
        current->setText(index < 0 ? QStringLiteral("选中:-")
                                   : QStringLiteral("选中:%1").arg(table->track(index).title));
    });
    connect(table, &PlaylistTable::activated, this, [table, current](int index) {
        table->setPlayingIndex(index);
        current->setText(QStringLiteral("播放:%1").arg(table->track(index).title));
        ToastLabel::showText(QStringLiteral("开始播放:") + table->track(index).title, table);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addWidget(table, 1);
    layout->addWidget(current);
}
