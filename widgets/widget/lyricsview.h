#pragma once

#include <QVariantAnimation>
#include <QVector>
#include <QWidget>

#include "lrcparser.h"

class QMouseEvent;
class QWheelEvent;

// 歌词视图:逐行歌词,当前行加粗放大、主文本色,其余淡色。
// 数据源是 LrcParser::parse() 的行序列;setTime(ms) 由播放进度驱动,
// 当前行切换时平滑滚动到视口中部;滚轮自由浏览,下一次行切换重新居中。
// 点击某行发 lineClicked(ms),业务侧用来 seek
class LyricsView : public QWidget
{
    Q_OBJECT
public:
    explicit LyricsView(QWidget *parent = nullptr);

    void setLines(const QVector<LrcLine> &lines);
    void clear();

    int lineCount() const;
    int currentLine() const;      // 当前行号,-1 无(进度未到第一行)
    qint64 lineTime(int index) const;   // 越界返回 -1(暴露给测试/业务)

    QSize sizeHint() const override;

signals:
    void lineClicked(qint64 ms);  // 点击行请求跳转

public slots:
    void setTime(qint64 ms);      // 进度驱动:高亮 <= ms 的最后一行

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    int lineAt(const QPoint &pos) const;
    int maxOffset() const;
    void scrollTo(int index);     // 动画居中到 index

    QVector<LrcLine> m_lines;
    int m_current = -1;
    qreal m_offset = 0;           // 内容纵向上移量(0 = 第一行在顶部)
    QVariantAnimation m_scroll;
};
