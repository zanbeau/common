#pragma once

#include <QString>
#include <QVector>

// LRC 歌词行:开始时间 + 文本
struct LrcLine
{
    qint64 ms = 0;
    QString text;   // 可为空(空行保留,排版占位)
};

// LRC 歌词解析(纯文本 -> 时间升序行序列),供 LyricsView / 业务侧使用:
//  - 时间标签:[mm:ss] / [mm:ss.z] / [mm:ss.zz] / [mm:ss.zzz](小数点或逗号)
//  - 一行多个时间标签([00:12.00][00:15.00]text)按同文本拆成多行
//  - 元数据:[ti:] 标题 / [ar:] 艺术家 / [al:] 专辑;[offset:±ms] 全局平移,
//    正值整体提前(生效时间 = 标签时间 - offset);其余标签忽略
//  - 无时间标签的行只认元数据,其余丢弃
class LrcParser
{
public:
    struct Result
    {
        QVector<LrcLine> lines;   // 已按时间升序(同时间保持出现顺序)
        QString title;            // [ti:]
        QString artist;           // [ar:]
        QString album;            // [al:]
    };

    static Result parse(const QString &lrcText);
};
