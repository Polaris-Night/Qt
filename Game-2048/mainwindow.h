#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include <QStack>
#include <QPoint>
#include <QTimer>
#include <QVector>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

#define ARRAYSIZE 4
#define MAXSTEPSAVESIZE 5

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void initConnect();//初始化连接关系
    void initTextEditArray();//初始化文本框控件，即将文本框控件存入二维指针数组
    void initTimer();//初始化定时
    void initGame();//初始化游戏
    void restartGame();//重新开始游戏
    int getRandomNumberPos1(int min, int max);//获取随机位置1
    int getRandomNumberPos2(int min, int max);//获取随机位置2
    void updateSource(QStack<int> &calculatedNumbers);//更新成绩
    void mend2or4(QPoint &pos);//随机位置生成2或4
    void saveStep();//步骤保存
    void checkGameOver(int &needNewRandom);//游戏判定

public slots:
    void pressUp();//上移
    void pressRight();//右移
    void pressDown();//下移
    void pressLeft();//左移
    void handletimeOut();//定时计数器溢出
    void saveSource();//保存成绩
    void showSource();//显示成绩
    void backStep();//步骤回退

private:
    Ui::MainWindow *ui;
    QTextEdit *textEditArray[ARRAYSIZE][ARRAYSIZE];//二维指针数组，存储文本框控件
    QTimer *countTime;//定时器指针
    QStack<QPoint> emptyElemPosition;//空位置栈
    int clickCount;//点击次数
    int timeOutCount;//时间计数
    int source;//成绩
    bool sourceIsSave;//成绩保存标志
    QStack<QVector<QVector<int> > > stepStack;//步骤栈
    QStack<int> sourceStack;//成绩栈

};
#endif // MAINWINDOW_H
