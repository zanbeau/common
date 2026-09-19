#pragma once

#include <QPushButton>

// 主题按钮:按 Type 呈现 Ant 式扁平风格(参考 Arco 色板),自绘实现,
// 颜色全部取自 Theme,亮暗主题自动联动:
//   PushButton("确定", PushButton::Type::Primary)
class PushButton : public QPushButton
{
    Q_OBJECT
public:
    enum class Type
    {
        Default,  // 白底描边
        Primary,  // 主色实底
        Danger,   // 危险操作实底
        Text      // 纯文字
    };

    explicit PushButton(const QString &text = {}, Type type = Type::Default,
                        QWidget *parent = nullptr);

    Type type() const;
    void setType(Type type);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Type m_type = Type::Default;
};
