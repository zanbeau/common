#pragma once

#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>

class QPainter;

// 无边框窗口的自绘外观:内容容器 + 四周阴影 + 圆角卡片,全平台视觉一致。
// 宿主窗口需开 WA_TranslucentBackground;本类过滤宿主的 Resize/WindowStateChange
// 维护内容几何,但绘制必须发生在宿主的 paintEvent 里(子控件永远画在宿主
// 自身绘制之上),宿主在 paintEvent 调 paint() 转发即可。
// 阴影预渲染成九宫格贴片(四角块 + 四边条,按宿主 DPR 出图后逐帧贴图),
// 半透明窗口拖拽/拉伸的每帧重绘不再反复栅格化八段渐变;贴片只随 DPR
// (跨屏)变化重建,与窗口尺寸无关
class FramelessShadow : public QObject
{
    Q_OBJECT
public:
    explicit FramelessShadow(QWidget *host);

    QWidget *contentWidget() const;      // 业务布局的宿主(透明容器)
    QVBoxLayout *contentLayout() const;  // contentWidget 上的 0 边距布局,直接 addWidget

    // 关 = 无阴影无内缩(内容铺满窗口,圆角卡仍在);默认开
    void setShadowEnabled(bool enabled);
    bool shadowEnabled() const;

    int shadowMargin() const;            // 常量 12,业务排布与测试参考

    void paint(QPainter *painter);       // 宿主 paintEvent 转发入口

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void layoutContent();
    void ensurePatches();                // 阴影贴片按当前 DPR 懒加载

    QWidget *m_host = nullptr;
    QWidget *m_content = nullptr;
    QVBoxLayout *m_layout = nullptr;
    bool m_shadowEnabled = true;
    QPixmap m_cornerTL, m_cornerTR, m_cornerBL, m_cornerBR;
    QPixmap m_stripTop, m_stripBottom, m_stripLeft, m_stripRight;
    qreal m_patchDpr = 0.0;
};
