#pragma once
#include <QWidget>

// LyricsView 演示页
class LyricsViewPage : public QWidget
{
    Q_OBJECT
public:
    explicit LyricsViewPage(QWidget *parent = nullptr);

private:
    qint64 m_ms = 0;   // 模拟播放进度
};
