#pragma once

#include <QString>
#include <QStringList>

// 播放时长格式化:毫秒 <-> "mm:ss"(不足 1 小时)/ "h:mm:ss"
// parse 接受 "s" / "mm:ss" / "h:mm:ss" 三种形式,非法输入返回 -1
namespace Duration {

inline QString format(qint64 ms)
{
    const qint64 total = qMax<qint64>(ms, 0) / 1000;
    const qint64 h = total / 3600;
    const qint64 m = (total % 3600) / 60;
    const qint64 s = total % 60;
    if(h > 0)
    {
        return QStringLiteral("%1:%2:%3")
            .arg(h)
            .arg(m, 2, 10, QLatin1Char('0'))
            .arg(s, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2")
        .arg(m, 2, 10, QLatin1Char('0'))
        .arg(s, 2, 10, QLatin1Char('0'));
}

inline qint64 parse(const QString &text)
{
    const QStringList parts = text.trimmed().split(QLatin1Char(':'));
    if(parts.isEmpty() || parts.size() > 3)
    {
        return -1;
    }

    qint64 total = 0;
    for(const QString &part : parts)
    {
        bool ok = false;
        const qint64 v = part.toLongLong(&ok);
        if(!ok || v < 0)
        {
            return -1;
        }
        total = total * 60 + v;
    }
    return total * 1000;
}

} // namespace Duration
