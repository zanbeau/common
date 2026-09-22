#include "lrcparser.h"

#include <QRegularExpression>

#include <algorithm>

namespace {

// 行首时间标签:[mm:ss] / [mm:ss.z..zzz](小数点或逗号);分秒位数宽容
const QRegularExpression kTimeTag(
    QStringLiteral("\\A\\[(\\d{1,3}):(\\d{1,2})(?:[.,](\\d{1,3}))?\\]"));

// 独立元数据行:[key:value](key 字母),offset/ti/ar/al 之外忽略
const QRegularExpression kMeta(
    QStringLiteral("\\A\\[([A-Za-z]+):([^\\]]*)\\]\\s*\\z"));

// 小数部分按位数补齐到毫秒:.5 = 500ms,.50 = 500ms,.500 = 500ms
qint64 tagToMs(int minute, int second, const QString &fraction)
{
    qint64 ms = (qint64(minute) * 60 + second) * 1000;
    QString padded = fraction;
    while(padded.size() < 3)
    {
        padded += QLatin1Char('0');
    }
    return ms + padded.toInt();
}

} // namespace

LrcParser::Result LrcParser::parse(const QString &lrcText)
{
    Result result;
    qint64 offset = 0;

    // 第一遍找 offset(全局生效,与所在行位置无关)
    const QStringList rows = lrcText.split(QRegularExpression(QStringLiteral("\\r\\n|\\r|\\n")));
    for(const QString &row : rows)
    {
        const QRegularExpressionMatch meta = kMeta.match(row);
        if(meta.hasMatch() && meta.captured(1).compare(QStringLiteral("offset"), Qt::CaseInsensitive) == 0)
        {
            bool ok = false;
            const qint64 value = meta.captured(2).trimmed().toLongLong(&ok);
            if(ok)
            {
                offset = value;
            }
        }
    }

    for(const QString &row : rows)
    {
        QRegularExpressionMatch tag = kTimeTag.match(row);
        if(!tag.hasMatch())
        {
            // 无时间标签:只取元数据,歌词外的杂行丢弃
            const QRegularExpressionMatch meta = kMeta.match(row);
            if(meta.hasMatch())
            {
                const QString key = meta.captured(1).toLower();
                const QString value = meta.captured(2).trimmed();
                if(key == QStringLiteral("ti"))
                {
                    result.title = value;
                }
                else if(key == QStringLiteral("ar"))
                {
                    result.artist = value;
                }
                else if(key == QStringLiteral("al"))
                {
                    result.album = value;
                }
            }
            continue;
        }

        // 逐个吃掉行首的时间标签,余下为文本
        QVector<qint64> stamps;
        QString rest = row;
        while(tag.hasMatch())
        {
            stamps.append(tagToMs(tag.captured(1).toInt(), tag.captured(2).toInt(),
                                  tag.captured(3)));
            rest = rest.mid(tag.capturedLength());
            tag = kTimeTag.match(rest);
        }
        const QString text = rest.trimmed();
        for(const qint64 stamp : stamps)
        {
            result.lines.append({stamp - offset, text});
        }
    }

    std::stable_sort(result.lines.begin(), result.lines.end(),
                     [](const LrcLine &a, const LrcLine &b) { return a.ms < b.ms; });
    return result;
}
