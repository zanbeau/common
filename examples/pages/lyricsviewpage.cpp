#include "lyricsviewpage.h"

#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QVBoxLayout>

#include "clickedslider.h"
#include "duration.h"
#include "lrcparser.h"
#include "lyricsview.h"
#include "pushbutton.h"
#include "toastlabel.h"

namespace {

// 演示歌词(自编):元数据 + offset + 每行时间标签
const char kSampleLrc[] = R"LRC([ti:晚风信笺]
[ar:演示歌手]
[al:控件库示例]
[offset:+500]
[00:00.00]晚风信笺 - 演示歌手
[00:04.50]
[00:08.00]路灯把影子拉得很长
[00:12.50]晚风翻动摊开的信笺
[00:17.00]字里行间都是旧时光
[00:21.50]落款写着一个夏天
[00:26.00]
[00:30.00]我把心事折成纸飞机
[00:34.50]让它替我说给你听
[00:39.00]如果它落在你的窗前
[00:43.50]就当月亮替我问了一声
[00:48.00]
[00:52.00]晚风啊 慢一点吹
[00:56.50]别惊动未说出口的话
[01:01.00]星光啊 亮一些吧
[01:05.50]照亮回家的那条街
[01:10.00]
[01:14.00]信笺的结尾没有署名
[01:18.50]只有一句 晚安 好梦
[01:23.00]晚安 好梦
[01:27.50])LRC";

constexpr qint64 kSongMs = 92000;   // 模拟歌曲总长

} // namespace

LyricsViewPage::LyricsViewPage(QWidget *parent)
    : QWidget(parent)
{
    QLabel *intro = new QLabel(QStringLiteral(
        "LyricsView:歌词视图——当前行加粗放大、主文本色,行切换平滑居中。\n"
        "数据来自 LrcParser(core/base);拖动进度条或点击歌词行可跳转,播放/暂停控制模拟进度。"));
    intro->setTextInteractionFlags(Qt::TextSelectableByMouse);

    LyricsView *view = new LyricsView;
    view->setLines(LrcParser::parse(QString::fromUtf8(kSampleLrc)).lines);

    ClickedSlider *seek = new ClickedSlider(Qt::Horizontal);
    seek->setRange(0, int(kSongMs));
    seek->setValue(0);

    PushButton *toggle = new PushButton(QStringLiteral("播放 / 暂停"), PushButton::Type::Primary);

    QTimer *ticker = new QTimer(this);
    ticker->setInterval(200);
    connect(ticker, &QTimer::timeout, this, [this, view, seek]() {
        m_ms = qMin<qint64>(m_ms + 200, kSongMs);
        view->setTime(m_ms);
        seek->blockSignals(true);
        seek->setValue(int(m_ms));
        seek->blockSignals(false);
        if(m_ms >= kSongMs)
        {
            m_ms = 0;
        }
    });
    connect(toggle, &PushButton::clicked, this, [ticker]() {
        ticker->isActive() ? ticker->stop() : ticker->start(200);
    });
    connect(seek, &ClickedSlider::sliderMoved, this, [this, view](int value) {
        m_ms = value;
        view->setTime(m_ms);
    });
    connect(view, &LyricsView::lineClicked, this, [this, view, seek](qint64 ms) {
        m_ms = ms;
        view->setTime(m_ms);
        seek->blockSignals(true);
        seek->setValue(int(m_ms));
        seek->blockSignals(false);
        ToastLabel::showText(QStringLiteral("跳转到 ") + Duration::format(ms), view);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->addWidget(intro);
    layout->addWidget(view, 1);
    layout->addWidget(seek);
    layout->addWidget(toggle, 0, Qt::AlignHCenter);
}
