#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QTime>
#include <QFile>
#include <QString>
#include <QDir>
#include <QTextCodec>
#include <QByteArray>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("GAME-2048");
    initTextEditArray();
    initGame();
    initTimer();
    initConnect();
}

//初始化连接关系
void MainWindow::initConnect()
{
    //菜单按钮相关
    connect(ui->buttonSaveSource, &QPushButton::clicked, this, &MainWindow::saveSource);
    connect(ui->buttonShowSource, &QPushButton::clicked, this, &MainWindow::showSource);

    connect(ui->buttonNewGame, &QPushButton::clicked, [=](){
        if (QMessageBox::Yes == QMessageBox::question(this, "Question Window", "Sure to start a new game?"
                                                      , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes)) {
            //未保存本轮成绩时询问是否保存成绩
            if (!this->sourceIsSave) {
                if (QMessageBox::Yes == QMessageBox::question(this, "Question Window", "Save the source of this round?"
                                                              , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes))
                    saveSource();
            }

            //游戏重新开始
            restartGame();
        }
    });

    connect(ui->buttonCloseGame, &QPushButton::clicked, [=](){
        if (QMessageBox::Yes == QMessageBox::question(this, "Question Window", "Sure to close the game?"
                                                              , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes)) {
            //未保存本轮成绩时询问是否保存成绩
            if (!this->sourceIsSave) {
                if (QMessageBox::Yes == QMessageBox::question(this, "Question Window", "Save the source of this round?"
                                                              , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes))
                    saveSource();
            }

            //关闭游戏窗口
            this->close();
        }
    });

    //操作按钮相关
    //上移
    connect(ui->buttonUp, &QToolButton::clicked, this, [=](){
        saveStep();
        pressUp();
    });
    //右移
    connect(ui->buttonRight, &QToolButton::clicked, this, [=](){
        saveStep();
        pressRight();
    });
    //下移
    connect(ui->buttonDown, &QToolButton::clicked, this, [=](){
        saveStep();
        pressDown();
    });
    //左移
    connect(ui->buttonLeft, &QToolButton::clicked, this, [=](){
        saveStep();
        pressLeft();
    });
    connect(ui->buttonBackStep, &QPushButton::clicked, this, &MainWindow::backStep);


    //绑定功能键事件
    ui->buttonShowSource->setShortcut(Qt::Key_H);
    ui->buttonSaveSource->setShortcut(Qt::Key_S);
    ui->buttonNewGame->setShortcut(Qt::Key_R);
    ui->buttonCloseGame->setShortcut(Qt::Key_C);
    ui->buttonBackStep->setShortcut(Qt::Key_B);

    //绑定方向键事件
    ui->buttonUp->setShortcut(Qt::Key_Up);
    ui->buttonRight->setShortcut(Qt::Key_Right);
    ui->buttonDown->setShortcut(Qt::Key_Down);
    ui->buttonLeft->setShortcut(Qt::Key_Left);

    //单行文本框只读
    ui->sourceLineEdit->setReadOnly(true);
    ui->timeLineEdit->setReadOnly(true);
}

//初始化文本框控件，即将文本框控件存入二维指针数组
void MainWindow::initTextEditArray()
{
    //将文本框控件存入指针数组
    textEditArray[0][0] = ui->num00;
    textEditArray[0][1] = ui->num01;
    textEditArray[0][2] = ui->num02;
    textEditArray[0][3] = ui->num03;

    textEditArray[1][0] = ui->num10;
    textEditArray[1][1] = ui->num11;
    textEditArray[1][2] = ui->num12;
    textEditArray[1][3] = ui->num13;

    textEditArray[2][0] = ui->num20;
    textEditArray[2][1] = ui->num21;
    textEditArray[2][2] = ui->num22;
    textEditArray[2][3] = ui->num23;

    textEditArray[3][0] = ui->num30;
    textEditArray[3][1] = ui->num31;
    textEditArray[3][2] = ui->num32;
    textEditArray[3][3] = ui->num33;

    //设置文本框控件只读
    for (int i = 0; i < ARRAYSIZE; i++) {
        for (int j = 0; j < ARRAYSIZE; j++) {
            textEditArray[i][j]->setReadOnly(true);
        }
    }
}

//初始化定时
void MainWindow::initTimer()
{
    this->countTime = new QTimer(this);
    connect(countTime, &QTimer::timeout, this, &MainWindow::handletimeOut);
    this->countTime->start(1000);
}

//初始化游戏
void MainWindow::initGame()
{
    //各项指标初始化
    this->sourceIsSave = false;
    this->clickCount = 0;
    this->source = 0;
    this->timeOutCount = 0;
    //清空容器
    this->stepStack.clear();
    this->sourceStack.clear();

    //textEditArray内容清空
    for (int i = 0; i < ARRAYSIZE; i++) {
        for (int j = 0; j < ARRAYSIZE; j++)
            textEditArray[i][j]->clear();
    }

    //textEditArray中随机取两个位置出现2
    int x = getRandomNumberPos1(0, 15);
    int y = getRandomNumberPos2(0, 15);

    //若随机位置相同则继续取随机数
    while (x == y) {
        x = getRandomNumberPos1(0, 15);
        y = getRandomNumberPos2(0, 15);
    }

    //在两个不同的随机位置生成两个2
    textEditArray[x/4][x%4]->setText(QString::number(2));
    textEditArray[y/4][y%4]->setText(QString::number(2));
    ui->sourceLineEdit->setText(QString::number(this->source));
    ui->timeLineEdit->setText(QString::number(this->timeOutCount));
}

//重新开始游戏
void MainWindow::restartGame()
{
    //初始化游戏和定时器
    if (this->countTime != nullptr) {
        delete this->countTime;
        this->countTime = nullptr;
    }
    initGame();
    initTimer();
}

//获取随机位置1
int MainWindow::getRandomNumberPos1(int min, int max)
{
    //以当前时间作为随机数种子，在min到max间取随机数
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int randomNum = qrand() % (max - min);
    return randomNum;
}

//获取随机位置2
int MainWindow::getRandomNumberPos2(int min, int max)
{
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime())*100);
    int randomNum = qrand() % (max - min);
    return randomNum;
}


//更新成绩
void MainWindow::updateSource(QStack<int> &calculatedNumbers)
{
    //计算成绩并更新
    while (!calculatedNumbers.isEmpty()) {
        this->source += calculatedNumbers.top();
        calculatedNumbers.pop();
    }
    ui->sourceLineEdit->setText(QString::number(this->source));
}

//随机位置生成2或4
void MainWindow::mend2or4(QPoint &pos)
{
    //每点击10次出一个4
    if (++this->clickCount % 10 == 0 && this->clickCount != 0) {
        textEditArray[pos.x()][pos.y()]->setText(QString::number(4));
        this->clickCount = 0;

    } else {
        textEditArray[pos.x()][pos.y()]->setText(QString::number(2));
    }
}

//步骤保存
void MainWindow::saveStep()
{
    //将步骤的值存储到容器
    QVector<QVector<int> > s1;
    QVector<int> s2;
    for (int i = 0; i < ARRAYSIZE; i++) {
        for (int j = 0; j < ARRAYSIZE; j++)
            s2.insert(j, textEditArray[i][j]->toPlainText().toInt());

        s1.insert(i, s2);
    }

    if (stepStack.size() >= MAXSTEPSAVESIZE) {
        QStack<QVector<QVector<int> > > tempStepStack;
        QStack<int> tempSourceStack;
        //临时栈容器存储顺序由底向顶为：新到旧
        for (int i = 0; i < MAXSTEPSAVESIZE-1; i++) {
            QVector<QVector<int> > tempStep = stepStack.top();
            int tempSource = sourceStack.top();
            stepStack.pop();
            sourceStack.pop();
            tempStepStack.push(tempStep);
            tempSourceStack.push(tempSource);
        }
        //清空栈容器
        stepStack.clear();
        sourceStack.clear();
        //临时栈栈顶元素入栈容器，栈容器存储顺序由底到顶为：旧到新
        for (int i = 0; i < MAXSTEPSAVESIZE-1; i++) {
            stepStack.push(tempStepStack.top());
            sourceStack.push(tempSourceStack.top());
            tempStepStack.pop();
            tempSourceStack.pop();
        }
    }

    //将最新一步入栈
    stepStack.push(s1);
    sourceStack.push(this->source);

//    for (int i = 0; i < ARRAYSIZE; i++) {
//        for (int j = 0; j < ARRAYSIZE; j++) {
//            qDebug() << i << " " << j << s1[i][j];
//        }
//    }
}

//游戏判定
void MainWindow::checkGameOver(int &needNewRandom)
{
    bool isFull = true;
    bool gameOver = true;

    //判断方格是否填满，未满则将isFull置为false
    for (int i = 0; i < ARRAYSIZE && isFull; i++) {
        for (int j = 0; j < ARRAYSIZE && isFull; j++) {
            if (textEditArray[i][j]->toPlainText().isEmpty())
                isFull = false;
        }
    }

    //已满的情况下判断是否还可以移动，可以移动则将gameOver置为false
    if (isFull) {
        for (int i = 0; i < ARRAYSIZE && gameOver; i++) {
            for (int j = 0; j < ARRAYSIZE && gameOver; j++) {

                if (i < ARRAYSIZE-1 && j < ARRAYSIZE-1) {//除最后一行和最后一列的元素外，其余元素与其右方和下方元素比较是否相同
                    if (textEditArray[i][j]->toPlainText().toInt() == textEditArray[i][j+1]->toPlainText().toInt()
                            || textEditArray[i][j]->toPlainText().toInt() == textEditArray[i+1][j]->toPlainText().toInt())
                        gameOver = false;
                } else if (i < ARRAYSIZE-1) {//最后一列的元素除末尾元素外，其余元素与其下方元素比较是否相同
                    if (textEditArray[i][j]->toPlainText().toInt() == textEditArray[i+1][j]->toPlainText().toInt())
                        gameOver = false;
                } else if (j < ARRAYSIZE-1) {//最后一行的元素除末尾元素外，其余元素与其右方元素比较是否相同
                    if (textEditArray[i][j]->toPlainText().toInt() == textEditArray[i][j+1]->toPlainText().toInt())
                        gameOver = false;
                }

            }
        }
    }

    //方格已满，且不能再移动，游戏结束
    if (isFull == true && gameOver == true && needNewRandom == 0) {
        if (QMessageBox::Yes == QMessageBox::information(this, "Information Window", "Sorry, you lose!\nWant to start a new game?"
                                                         , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel)) {
            //未保存本轮成绩时询问是否保存成绩
            if (!this->sourceIsSave) {
                if (QMessageBox::Yes == QMessageBox::question(this, "Question Window", "Save the source of this round?"
                                                              , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes))
                    saveSource();
            }

            //游戏重新开始
            restartGame();
        }
    }
}

//上移
void MainWindow::pressUp()
{
    int needNewRandom = 0;
    QStack<int> calculatedNumbers;

    //上移，按列操作，即外循环操作列，内循环操作行
    for (int j = 0; j < ARRAYSIZE; j++) {//总共操作ARRAYSIZE列，即4列
        for (int k = 0; k < ARRAYSIZE-1; k++) {//每个元素最多上移ARRAYSIZE-1次，即3次
            for (int i = 0; i < ARRAYSIZE-1; i++) {//每列第一个元素不用移动，则每列有ARRAYSIZE-1个元素移动
                if (textEditArray[i][j]->toPlainText().isEmpty()) {//当前元素值为空时将同列下一元素上移，然后将同列下一元素置空
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i+1][j] << textEditArray[i+1][j]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i+1][j]->toPlainText());
                    textEditArray[i+1][j]->setText(QString());
                    needNewRandom++;
                }
            }
        }
    }

    //计算
    for (int j = 0; j < ARRAYSIZE; j++) {//总共计算ARRAYSIZE列，即4列
        for (int i = 0; i < ARRAYSIZE-1; i++) {//每列最后一个元素不用计算，则有ARRAYSIZE-1个元素计算
            if(textEditArray[i][j]->toPlainText() == textEditArray[i+1][j]->toPlainText()
                    && !textEditArray[i][j]->toPlainText().isEmpty()) {//上下两元素值相等且不为空时将两元素值转整型相加后赋给当前元素，并将同列下一元素值置空
                textEditArray[i][j]->setText(QString::number(textEditArray[i][j]->toPlainText().toInt()
                                                             + textEditArray[i+1][j]->toPlainText().toInt()));
                textEditArray[i+1][j]->setText(QString());
                calculatedNumbers.push(textEditArray[i][j]->toPlainText().toInt());
                needNewRandom++;
            }
        }
    }

    //上移：计算后若有值的两个元素间有空位，则将下面元素上移，例：计算后若00=8(原为4，计算后为8)，10=0(原为4，计算后被置0)，20=2时，则将值2上移到10
    for(int j = 0; j < ARRAYSIZE; j++) {
        for(int k = 0; k < ARRAYSIZE-1; k++) {
            for(int i = 0; i < ARRAYSIZE-1; i++) {
                if(textEditArray[i][j]->toPlainText().isEmpty()
                        && !textEditArray[i+1][j]->toPlainText().isEmpty()) {
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i+1][j] << textEditArray[i+1][j]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i+1][j]->toPlainText());
                    textEditArray[i+1][j]->setText(QString());
                }
            }
        }
    }

    //在空位置的随机位置产生一个2或4
    if (needNewRandom > 0) {
        emptyElemPosition.clear();
        //保存空元素位置：将空元素位置入栈
        for (int i = 0; i < ARRAYSIZE; i++) {
            for (int j = 0; j < ARRAYSIZE; j++) {
                if (textEditArray[i][j]->toPlainText().isEmpty())
                    emptyElemPosition.push(QPoint(i, j));
            }
        }

        //统计空元素数量，并在其中随机取一个产生2或4
        int emptyElemCount = emptyElemPosition.count();
        int x = getRandomNumberPos2(0, emptyElemCount);
        //取随机位置：元素出栈x个元素
        for (int i = 0; i < x; i++)
            emptyElemPosition.pop();
        QPoint pos = emptyElemPosition.top();
        //补充2或4
        mend2or4(pos);
    }

    //更新成绩
    updateSource(calculatedNumbers);
    checkGameOver(needNewRandom);
}

//右移
void MainWindow::pressRight()
{
    int needNewRandom = 0;
    QStack<int> calculatedNumbers;

    //右移，按行操作，即外循环操作行，内循环操作列
    for (int i = 0; i < ARRAYSIZE; i++) {//总共操作ARRAYSIZE行，即4行
        for (int k = 0; k < ARRAYSIZE-1; k++) {//每个元素最多右移ARRAYSIZE-1次，即3次
            for (int j = ARRAYSIZE-1; j > 0; j--) {//每行最后一个元素不用移动，则每行有ARRAYSIZE-1个元素移动
                if (textEditArray[i][j]->toPlainText().isEmpty()) {//当前元素值为空时将同行前一元素右移，然后将同行前一元素置空
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i][j-1] << textEditArray[i][j-1]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i][j-1]->toPlainText());
                    textEditArray[i][j-1]->setText(QString());
                    needNewRandom++;
                }
            }
        }
    }

    //计算
    for (int i = 0; i < ARRAYSIZE; i++) {//总共计算ARRAYSIZE行，即4行
        for (int j = ARRAYSIZE-1; j > 0; j--) {//每行第一个元素不用计算，则有ARRAYSIZE-1个元素计算
            if(textEditArray[i][j]->toPlainText() == textEditArray[i][j-1]->toPlainText()
                    && !textEditArray[i][j]->toPlainText().isEmpty()) {//左右两元素值相等且不为空时将两元素值转整型相加后赋给当前元素，并将同行前一元素值置空
                textEditArray[i][j]->setText(QString::number(textEditArray[i][j]->toPlainText().toInt()
                                                             + textEditArray[i][j-1]->toPlainText().toInt()));
                textEditArray[i][j-1]->setText(QString());
                calculatedNumbers.push(textEditArray[i][j]->toPlainText().toInt());
                needNewRandom++;
            }
        }
    }

    //右移：计算后若有值的两个元素间有空位，则将前面元素右移，例：计算后若01=2，02=0(原为4，计算后被置0)，03=8(原为4，计算后为8)时，则将值2右移到02
    for (int i = 0;  i < ARRAYSIZE; i++) {
        for (int k = 0; k < ARRAYSIZE-1; k++) {
            for (int j = ARRAYSIZE-1; j > 0; j--) {
                if(textEditArray[i][j]->toPlainText().isEmpty()
                        && !textEditArray[i][j-1]->toPlainText().isEmpty()) {
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i][j-1] << textEditArray[i][j-1]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i][j-1]->toPlainText());
                    textEditArray[i][j-1]->setText(QString());
                }
            }
        }
    }

    //在空位置的随机位置产生一个2或4
    if (needNewRandom > 0) {
        emptyElemPosition.clear();
        //保存空元素位置：将空元素位置入栈
        for (int i = 0; i < ARRAYSIZE; i++) {
            for (int j = 0; j < ARRAYSIZE; j++) {
                if (textEditArray[i][j]->toPlainText().isEmpty())
                    emptyElemPosition.push(QPoint(i, j));
            }
        }

        //统计空元素数量，并在其中随机取一个产生2或4
        int emptyElemCount = emptyElemPosition.count();
        int x = getRandomNumberPos2(0, emptyElemCount);
        //取随机位置：元素出栈x个元素
        for (int i = 0; i < x; i++)
            emptyElemPosition.pop();
        QPoint pos = emptyElemPosition.top();
        //补充2或4
        mend2or4(pos);
    }

    //更新成绩
    updateSource(calculatedNumbers);
    checkGameOver(needNewRandom);
}

//下移
void MainWindow::pressDown()
{
    int needNewRandom = 0;
    QStack<int> calculatedNumbers;

    //下移，按列操作，即外循环操作列，内循环操作行
    for (int j = 0; j < ARRAYSIZE; j++) {//总共操作ARRAYSIZE列，即4列
        for (int k = 0; k < ARRAYSIZE-1; k++) {//每个元素最多下移ARRAYSIZE-1次，即3次
            for (int i = ARRAYSIZE-1;  i > 0; i--) {//每列最后一个元素不用移动，则每列有ARRAYSIZE-1个元素移动
                if (textEditArray[i][j]->toPlainText().isEmpty()) {//当前元素值为空时将同列上一元素下移，然后将同列上一元素置空
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i-1][j] << textEditArray[i-1][j]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i-1][j]->toPlainText());
                    textEditArray[i-1][j]->setText(QString());
                    needNewRandom++;
                }
            }
        }
    }

    //计算
    for (int j = 0; j < ARRAYSIZE; j++) {//总共计算ARRAYSIZE列，即4列
        for (int i = ARRAYSIZE-1; i > 0; i--) {//每列第一个元素不用计算，则有ARRAYSIZE-1个元素计算
            if(textEditArray[i][j]->toPlainText() == textEditArray[i-1][j]->toPlainText()
                    && !textEditArray[i][j]->toPlainText().isEmpty()) {//上下两元素值相等且不为空时将两元素值转整型相加后赋给当前元素，并将同列上一元素值置空
                textEditArray[i][j]->setText(QString::number(textEditArray[i][j]->toPlainText().toInt()
                                                             + textEditArray[i-1][j]->toPlainText().toInt()));
                textEditArray[i-1][j]->setText(QString());
                calculatedNumbers.push(textEditArray[i][j]->toPlainText().toInt());
                needNewRandom++;
            }
        }
    }

    //下移：计算后若有值的两个元素间有空位，则将上面元素下移，例：计算后若10=2，20=0(原为4，计算后被置0)，30=8(原为4，计算后为8)时，则将值2下移到20
    for(int j = 0; j < ARRAYSIZE; j++) {
        for(int k = 0; k < ARRAYSIZE-1; k++) {
            for(int i = ARRAYSIZE-1; i > 0; i--) {
                if(textEditArray[i][j]->toPlainText().isEmpty()
                        && !textEditArray[i-1][j]->toPlainText().isEmpty()) {
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i-1][j] << textEditArray[i-1][j]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i-1][j]->toPlainText());
                    textEditArray[i-1][j]->setText(QString());
                }
            }
        }
    }

    //在空位置的随机位置产生一个2或4
    if (needNewRandom > 0) {
        emptyElemPosition.clear();
        //保存空元素位置：将空元素位置入栈
        for (int i = 0; i < ARRAYSIZE; i++) {
            for (int j = 0; j < ARRAYSIZE; j++) {
                if (textEditArray[i][j]->toPlainText().isEmpty())
                    emptyElemPosition.push(QPoint(i, j));
            }
        }

        //统计空元素数量，并在其中随机取一个产生2或4
        int emptyElemCount = emptyElemPosition.count();
        int x = getRandomNumberPos2(0, emptyElemCount);
        //取随机位置：元素出栈x个元素
        for (int i = 0; i < x; i++)
            emptyElemPosition.pop();
        QPoint pos = emptyElemPosition.top();
        //补充2或4
        mend2or4(pos);
    }

    //更新成绩
    updateSource(calculatedNumbers);
    checkGameOver(needNewRandom);
}

//左移
void MainWindow::pressLeft()
{
    int needNewRandom = 0;
    QStack<int> calculatedNumbers;

    //左移，按行操作，即外循环操作行，内循环操作列
    for (int i = 0; i < ARRAYSIZE; i++) {//总共操作ARRAYSIZE行，即4行
        for (int k = 0; k < ARRAYSIZE-1; k++) {//每个元素最多左移ARRAYSIZE-1次，即3次
            for (int j = 0; j < ARRAYSIZE-1; j++) {//每行第一个元素不用移动，则每行有ARRAYSIZE-1个元素移动
                if (textEditArray[i][j]->toPlainText().isEmpty()) {//当前元素值为空时将同行后一元素左移，然后将同行后一元素置空
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i][j+1] << textEditArray[i][j+1]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i][j+1]->toPlainText());
                    textEditArray[i][j+1]->setText(QString());
                    needNewRandom++;
                }
            }
        }
    }

    //计算
    for (int i = 0; i < ARRAYSIZE; i++) {//总共计算ARRAYSIZE行，即4行
        for (int j = 0; j < ARRAYSIZE-1; j++) {//每行最后一个元素不用计算，则有ARRAYSIZE-1个元素计算
            if(textEditArray[i][j]->toPlainText() == textEditArray[i][j+1]->toPlainText()
                    && !textEditArray[i][j]->toPlainText().isEmpty()) {//左右两元素值相等且不为空时将两元素值转整型相加后赋给当前元素，并将同行前一元素值置空
                textEditArray[i][j]->setText(QString::number(textEditArray[i][j]->toPlainText().toInt()
                                                             + textEditArray[i][j+1]->toPlainText().toInt()));
                textEditArray[i][j+1]->setText(QString());
                calculatedNumbers.push(textEditArray[i][j]->toPlainText().toInt());
                needNewRandom++;
            }
        }
    }

    //左移：计算后若有值的两个元素间有空位，则将前面元素左移，例：计算后若00=8(原为4，计算后为8)，01=0，02=2时，则将值2右移到01
    for(int i = 0; i < ARRAYSIZE; i++) {
        for(int k = 0; k < ARRAYSIZE-1; k++) {
            for(int j = 0; j < ARRAYSIZE-1; j++) {
                if(textEditArray[i][j]->toPlainText().isEmpty()
                        && !textEditArray[i][j+1]->toPlainText().isEmpty()) {
                    //qDebug() << textEditArray[i][j] << "->setText" << textEditArray[i][j+1] << textEditArray[i][j+1]->toPlainText();
                    textEditArray[i][j]->setText(textEditArray[i][j+1]->toPlainText());
                    textEditArray[i][j+1]->setText(QString());
                }
            }
        }
    }

    //在空位置的随机位置产生一个2或4
    if (needNewRandom > 0) {
        emptyElemPosition.clear();
        //保存空元素位置：将空元素位置入栈
        for (int i = 0; i < ARRAYSIZE; i++) {
            for (int j = 0; j < ARRAYSIZE; j++) {
                if (textEditArray[i][j]->toPlainText().isEmpty())
                    emptyElemPosition.push(QPoint(i, j));
            }
        }

        //统计空元素数量，并在其中随机取一个产生2或4
        int emptyElemCount = emptyElemPosition.count();
        int x = getRandomNumberPos2(0, emptyElemCount);
        //取随机位置：元素出栈x个元素
        for (int i = 0; i < x; i++)
            emptyElemPosition.pop();
        QPoint pos = emptyElemPosition.top();

        mend2or4(pos);//补充0或4
    }
    //更新成绩
    updateSource(calculatedNumbers);
    checkGameOver(needNewRandom);
}

//定时计数器溢出
void MainWindow::handletimeOut()
{
    this->timeOutCount++;
    ui->timeLineEdit->setText(QString::number(this->timeOutCount));
}

//保存成绩
void MainWindow::saveSource()
{
    QString filePath = QDir::currentPath() + "/HistorySource.txt";
    QFile file(filePath);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    QString sourceString = "Source:" + ui->sourceLineEdit->text() + ",Time:" + ui->timeLineEdit->text()
            + ",Date:" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh::mm::ss") + "\n";

    file.write(sourceString.toUtf8());
    file.close();

    this->sourceIsSave = true;
}

//显示成绩
void MainWindow::showSource()
{
    //以只读方式打开文件
    QString filePath = QDir::currentPath() + "/HistorySource.txt";
    QFile file(filePath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    //创建浏览窗口及其关闭按钮
    QTextEdit *sourceEdit = new QTextEdit(this);
    QPushButton *closeEdit = new QPushButton("CLOSE", sourceEdit);
    connect(closeEdit, &QPushButton::clicked, [=](){
        sourceEdit->close();
        delete sourceEdit;
    });
    sourceEdit->setGeometry(250, 0, 500, 500);
    closeEdit->setGeometry(400, 0, 70, 50);
    sourceEdit->show();

    QByteArray readSource;
    QTextCodec *codec = QTextCodec::codecForName("gbk");

    while (!file.atEnd()) {
        readSource = file.readLine();
        sourceEdit->append(codec->toUnicode(readSource));
    }

    file.close();
}

//步骤回退
void MainWindow::backStep()
{
    //步骤存储栈为空则弹窗提示
    if (stepStack.isEmpty()) {
        QMessageBox::information(this, "Information Window", "The maximum step has been reached!");
        return;
    }

    //将步骤存储栈的栈顶元素赋给步骤操作变量step，将成绩存储栈的栈顶元素直接赋给成绩变量
    QVector<QVector<int> > step = stepStack.top();
    this->source = sourceStack.top();
    //步骤及成绩存储栈出掉栈顶元素
    stepStack.pop();
    sourceStack.pop();

    //将步骤操作变量step的值还原到方格中
    for (int i = 0; i < ARRAYSIZE; i++) {
        for (int j = 0; j < ARRAYSIZE; j++) {
            if (step[i][j] == 0)
                textEditArray[i][j]->setText(QString());
            else
                textEditArray[i][j]->setText(QString::number(step[i][j]));
        }
    }
    ui->sourceLineEdit->setText(QString::number(this->source));
}

MainWindow::~MainWindow()
{
    delete ui;
}
