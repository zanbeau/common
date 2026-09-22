#pragma once

#include <QAbstractScrollArea>
#include <QPixmap>
#include <QVector>

class QKeyEvent;
class QMouseEvent;
class QPainter;
class QResizeEvent;

// 播放列表:自绘行列表——序号 / 封面缩略 / 标题+艺术家 / 时长,视口虚拟化绘制
// (只画可见行,万级行滚动无压力)。
//  - 单选:单击或 上/下/PgUp/PgDn/Home/End 选中(currentChanged);
//    双击/回车激活(activated,业务侧开播)
//  - 正在播放行:setPlayingIndex() 主色淡底 + 左侧指示条 + 序号位换播放字形,
//    标题转主色;与选中态相互独立
//  - 封面缩略按行高懒缩放缓存(首次画到才缩放);时长列 Duration::format
// 数据为一次性设入的值拷贝;setTracks 会复位选中/播放行
class PlaylistTable : public QAbstractScrollArea
{
    Q_OBJECT
public:
    struct Track
    {
        QString title;
        QString artist;
        qint64 duration = 0;   // 毫秒
        QPixmap cover;         // 可空;列表存在任一非空封面时启用缩略图列
    };

    explicit PlaylistTable(QWidget *parent = nullptr);

    void setTracks(const QVector<Track> &tracks);
    int count() const;
    Track track(int index) const;         // 越界返回空 Track

    int currentIndex() const;             // 选中行,-1 无
    void setCurrentIndex(int index);      // 越界(含 -1)清除选中

    int playingIndex() const;             // 正在播放行,-1 无
    void setPlayingIndex(int index);

    int rowHeight() const;
    void setRowHeight(int height);        // 默认 44,最小 28

    QSize sizeHint() const override;

signals:
    void currentChanged(int index);       // 选中行变化
    void activated(int index);            // 双击/回车:请求播放该行

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    int rowAt(const QPoint &pos) const;   // 视口坐标 -> 行号,空白/越界 -1
    QRect rowRect(int index) const;       // 行的视口矩形
    void ensureVisible(int index);
    void updateScrollRange();
    QPixmap thumb(int index) const;       // 封面缩略(懒缩放缓存)

    QVector<Track> m_tracks;
    mutable QVector<QPixmap> m_thumbs;    // 与 m_tracks 对齐,按需填充
    bool m_hasCovers = false;
    int m_current = -1;
    int m_playing = -1;
    int m_hover = -1;
    int m_rowHeight = 44;
};
