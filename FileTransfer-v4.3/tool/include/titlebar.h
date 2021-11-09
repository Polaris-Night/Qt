#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

QT_BEGIN_NAMESPACE
namespace Ui { class TitleBar; }
QT_END_NAMESPACE

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

    /**
     * @brief msetStyleSheet 通过文件设置全局样式
     * @param styleSheet css/qss等样式文件
     */
    static void msetStyleSheet(const QString &styleSheet);

    /**
     * @brief setMainWidget 设置主窗口
     * @param widget 窗口指针
     */
    void setMainWidget(QWidget *widget);

    /**
     * @brief setTitleBarText 设置标题
     * @param text 文本
     */
    void setTitleBarText(const QString &text);

    /**
     * @brief setTitleBarIcon 设置标题栏图标
     * @param icon 图标
     */
    void setTitleBarIcon(const QString &icon);

    /**
     * @brief setTitleBarStyleSheet 设置标题栏样式
     * @param sheet 样式
     */
    void setTitleBarStyleSheet(const QString &sheet);

    /**
     * @brief setTitleBarBackGround 设置标题栏背景颜色
     * @param r 红色值
     * @param g 绿色值
     * @param b 蓝色值
     */
    void setTitleBarBackGround(const int &r, const int &g, const int &b);

    /**
     * @brief setTitleTextFont 设置标题栏字体
     * @param font 字体样式
     */
    void setTitleTextFont(const QFont &font);

    /**
     * @brief setMarginSize 设置拉伸边距
     * @param size 边距值
     */
    void setMarginSize(const int &size);

    /**
     * @brief The MousePosition enum 鼠标区域枚举
     * 左上角(1,1)   上(1,2)   右上角(1,3)
     * 左(2,1)      中(2,2)   右(2,3)
     * 左下角(3,1)  下(3,2)   右下角(3,3)
     */
    enum MousePosition
    {
        PosLeftTop = 11,
        PosTop = 12,
        PosRightTop = 13,
        PosLeft = 21,
        PosMid = 22,
        PosRight = 23,
        PosLeftBottom = 31,
        PosBottom = 32,
        PosRightBottom = 33,
    };

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    /**
     * @brief getMouseArea 获取鼠标所在区域
     * @param pos 全局坐标
     * @return
     */
    int getMouseArea(const QPoint &pos);

    /**
     * @brief setMouseCursor 设置鼠标样式
     * @param pos 全局坐标
     */
    void setMouseCursor(const QPoint &pos);

private:
    Ui::TitleBar *ui;

    QWidget *mainWidget;//主窗口指针
    QPoint dragPos;//窗口拖动位置
    bool isMoveEvent;//标题栏移动事件标志

    QPoint lastPos;//左键按下时停留的坐标
    bool leftBtnPress;//左键按下标志
    int mousePressArea; //鼠标点击的区域
    int marginSize;//边缘边距值
    bool isMidArea;//在非边缘区域标志
    int minWidth;//窗体(可缩小至)宽度
    int minHeight;//窗体(可缩小至)高度
};
#endif // TITLEBAR_H
