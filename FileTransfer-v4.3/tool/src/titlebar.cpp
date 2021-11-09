#include "titlebar.h"
#include "ui_titlebar.h"
#include <QStyle>
#include <QDebug>

TitleBar::TitleBar(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TitleBar)
{
    ui->setupUi(this);

    //窗体设置
    setWindowFlags(Qt::FramelessWindowHint | windowFlags());//隐藏默认标题栏
    //setAttribute(Qt::WA_TranslucentBackground, true);//背景透明
    setMouseTracking(true);//开启鼠标追踪
    ui->titleWidget->installEventFilter(this);//标题栏安装事件过滤器

    //默认值初始化
    isMoveEvent = false;//移动事件标志
    leftBtnPress = false;//左键按下标志
    mousePressArea = PosMid; //鼠标点击的区域
    marginSize = 5;//边缘边距值
    isMidArea = true;//非边缘区域标志
    minWidth = width();//窗体(可缩小至)宽度
    minHeight = height();//窗体(可缩小至)高度

    //标题栏按钮图标
    ui->btnMin->setIcon(style()->standardIcon(QStyle::SP_TitleBarMinButton));
    ui->btnMax->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
    ui->btnClose->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton));

    //按钮绑定
    connect(ui->btnMin, &QToolButton::clicked, this, &TitleBar::showMinimized);
    connect(ui->btnMax, &QToolButton::clicked, this, [=](){
        if (isMaximized()) {
            showNormal();
            ui->btnMax->setIcon(style()->standardIcon(QStyle::SP_TitleBarMaxButton));
        } else {
            showMaximized();
            ui->btnMax->setIcon(style()->standardIcon(QStyle::SP_TitleBarNormalButton));
        }
    });
    connect(ui->btnClose, &QToolButton::clicked, this, &TitleBar::close);

    //接口测试
//    setTitleTextFont(QFont("微软雅黑", 10));
//    setTitleBarIcon("test.png");
//    setTitleBarText("自定义标题栏测试");
//    setTitleBarBackGround(155, 100, 63);
//    msetStyleSheet("dark.css");
}

TitleBar::~TitleBar()
{
    delete ui;
}

/**********************************接口函数****************************************/
void TitleBar::msetStyleSheet(const QString &styleSheet)
{
    QFile sheetFile(styleSheet);
    if(sheetFile.open(QFile::ReadOnly)) {
        QString sheet = QLatin1String(sheetFile.readAll());
        qApp->setStyleSheet(sheet);
        sheetFile.close();
    }
}

void TitleBar::setMainWidget(QWidget *widget)
{
    if (widget != nullptr) {
        mainWidget = widget;
        mainWidget->setParent(this);
        mainWidget->show();
        mainWidget->installEventFilter(this);
        //重新布局标题栏和窗口
        QLayout *mainLayout = this->layout();
        mainLayout->removeWidget(ui->titleWidget);
        mainLayout->addWidget(ui->titleWidget);
        mainLayout->addWidget(mainWidget);
    }
}

void TitleBar::setTitleBarText(const QString &text)
{
    ui->titleText->setText(text);
    setWindowTitle(text);
}

void TitleBar::setTitleBarIcon(const QString &icon)
{
    ui->titleIcon->setPixmap(QPixmap(icon).scaled(25, 25));
}

void TitleBar::setTitleBarStyleSheet(const QString &sheet)
{
    ui->titleWidget->setStyleSheet(sheet);
}

void TitleBar::setTitleBarBackGround(const int &r, const int &g, const int &b)
{
    ui->titleWidget->setStyleSheet(QString("QWidget{background-color:rgb(%1, %2, %3);}").arg(r).arg(g).arg(b));
}

void TitleBar::setTitleTextFont(const QFont &font)
{
    ui->titleText->setFont(font);
}

void TitleBar::setMarginSize(const int &size)
{
    marginSize = size;
}

/**********************************鼠标事件函数**************************************/
int TitleBar::getMouseArea(const QPoint &pos)
{
    int posX = pos.x();//全局x坐标
    int posY = pos.y();//全局y坐标
    int mainWidth = width();//全局宽度
    int mainHeight = height();//全局高度
    int areaX = 0;//x所在区域
    int areaY = 0;//y所在区域

    //判断x所在区域
    if (posX > (mainWidth - marginSize))
        areaX = 3;
    else if (posX < marginSize)
        areaX = 1;
    else
        areaX = 2;

    //判断y所在区域
    if (posY > (mainHeight - marginSize))
        areaY = 3;
    else if (posY < marginSize)
        areaY = 1;
    else
        areaY = 2;

    //返回区域值，如区域1：1 + 1*10 = 11
    return areaX + areaY*10;
}

void TitleBar::setMouseCursor(const QPoint &pos)
{
    Qt::CursorShape cursor = Qt::ArrowCursor;
    //获取区域值
    int area = getMouseArea(pos);
    switch (area) {
        case PosLeftTop:
        case PosRightBottom:
            cursor = Qt::SizeFDiagCursor;
            break;
        case PosRightTop:
        case PosLeftBottom:
            cursor = Qt::SizeBDiagCursor;
            break;
        case PosLeft:
        case PosRight:
            cursor = Qt::SizeHorCursor;
            break;
        case PosTop:
        case PosBottom:
            cursor = Qt::SizeVerCursor;
            break;
        case PosMid:
            cursor = Qt::ArrowCursor;
            break;
        default:
            break;
    }
    setCursor(cursor);
}

bool TitleBar::eventFilter(QObject *watched, QEvent *event)
{
    //标题栏区域事件
    if (isMidArea && watched == ui->titleWidget) {
        setCursor(Qt::ArrowCursor);
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        switch (event->type()) {
            case QEvent::MouseButtonPress:
                dragPos = mouseEvent->globalPos() - frameGeometry().topLeft();//计算偏移量
                isMoveEvent = true;
                break;
            case QEvent::MouseButtonDblClick:
                isMoveEvent = false;
                emit ui->btnMax->clicked();
                break;
            case QEvent::MouseMove:
                if (isMoveEvent && !isMaximized())
                    move(mouseEvent->globalPos() - dragPos);//移动窗口
                break;
            case QEvent::MouseButtonRelease:
                isMoveEvent = false;
                break;
            default:
                break;
        }
    }
    //主窗口区域事件
    if (isMidArea && watched == mainWidget)
        setCursor(Qt::ArrowCursor);

    return QWidget::eventFilter(watched, event);
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        leftBtnPress = true;//左键按下标志
        lastPos = event->globalPos();//按下时鼠标坐标
        mousePressArea = getMouseArea(event->pos());//按下时鼠标所在区域
    }
    return QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
    //最大化状态时不能拉伸
    if (isMaximized())
        return;
    //根据位置设置鼠标样式
    if (!leftBtnPress)
        setMouseCursor(event->pos());

    if (leftBtnPress && (event->buttons() & Qt::LeftButton)) {
        if (mousePressArea != PosMid)
            isMidArea = false;
        QPoint offset = event->globalPos() - lastPos;//偏移量=当前鼠标坐标-按下时鼠标坐标
        int offsetX = offset.x();
        int offsetY = offset.y();
        QRect globalGeometry = frameGeometry();//获取窗口矩形
        int globalWidth = globalGeometry.width();
        int globalHeight = globalGeometry.height();
        //拉伸后位置=当前位置+偏移量
        //加入条件判断防止过度拉伸
        switch (mousePressArea) {
            case PosLeftTop:
                if (minWidth <= (globalWidth - offsetX))
                    globalGeometry.setLeft(globalGeometry.left() + offsetX);
                if (minHeight <= (globalHeight - offsetY))
                    globalGeometry.setTop(globalGeometry.top() + offsetY);
                //globalGeometry.setTopLeft(globalGeometry.topLeft() + offset);
                break;
            case PosTop:
                if (minHeight <= (globalHeight - offsetY))
                    globalGeometry.setTop(globalGeometry.top() + offsetY);
                break;
            case PosRightTop:
                if (minWidth <= (globalWidth + offsetX))
                    globalGeometry.setRight(globalGeometry.right() + offsetX);
                if (minHeight <= (globalHeight - offsetY))
                    globalGeometry.setTop(globalGeometry.top() + offsetY);
                //globalGeometry.setTopRight(globalGeometry.topRight() + offset);
                break;
            case PosRight:
                if (minWidth <= (globalWidth + offsetX))
                    globalGeometry.setRight(globalGeometry.right() + offsetX);
                break;
            case PosRightBottom:
                if (minWidth <= (globalWidth + offsetX))
                    globalGeometry.setRight(globalGeometry.right() + offsetX);
                if (minHeight <= globalHeight + offsetY)
                    globalGeometry.setBottom(globalGeometry.bottom() + offsetY);
                //globalGeometry.setBottomRight(globalGeometry.bottomRight() + offset);
                break;
            case PosBottom:
                if (minHeight <= (globalHeight + offsetY))
                    globalGeometry.setBottom(globalGeometry.bottom() + offsetY);
                break;
            case PosLeftBottom:
                if (minWidth <= (globalWidth - offsetX))
                    globalGeometry.setLeft(globalGeometry.left() + offsetX);
                if (minHeight <= (globalHeight + offsetY))
                    globalGeometry.setBottom(globalGeometry.bottom() + offsetY);
                //globalGeometry.setBottomLeft(globalGeometry.bottomLeft() + offset);
                break;
            case PosLeft:
                if (minWidth <= (globalWidth - offsetX))
                    globalGeometry.setLeft(globalGeometry.left() + offsetX);
                break;
            default:
                break;
        }
        //设置拉伸后坐标并记录
        setGeometry(globalGeometry);
        lastPos = event->globalPos();
    }

    return QWidget::mouseMoveEvent(event);
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    leftBtnPress = false;//左键释放
    isMidArea = true;
    setCursor(Qt::ArrowCursor);
    return QWidget::mouseReleaseEvent(event);
}
