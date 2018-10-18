#include "mywidget.h"
#include "ui_mywidget.h"
#include <QDebug>
#include <QThread>
#include <QPushButton>
#include <QIcon>
#include <QFileDialog>
#include <QFile>
#include <QTextCursor>
#include <QStringList>
#include <QVector>
#include <math.h>
#include <QMovie>
#include <QTime>
#include <QDate>
#define cout  qDebug() << __LINE__ << ":"
MyWidget::MyWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyWidget)
{
    //变量初始化
    x.fill(0, 40);
    y.fill(0, 40);
    for(int i = 0; i < 40; i++)
    {
        x[i] = i;
    }

    whochat = WHOCHAT::COM;
   // setContextMenuPolicy( Qt::NoContextMenu );
    /********界面初始化**********/
    ui->setupUi(this);
    setupRealtimeDataDemo(ui->customplot);
    this->setWindowTitle("SerialPort_v1.0");
    this->setWindowIcon(QIcon(QString(":/icon/Port1.ico")));        //设置程序图标
    ui->comboBox_databit->setCurrentIndex(3);       //设置当前数据位为8位
    labelmovie = new QMovie(this);       //创建图标movie对象
    labelmovie->setFileName(QString(":/movie/myserialport.gif"));
    ui->label_switchicon->setMovie(labelmovie);     //设置label动画
    labelmovie->start();
    labelmovie->stop();     //显示动画初始状态
    //ui->label_switchicon->setPixmap(QString(":/icon/Port1.ico"));       //设置开关label图标
    ui->label_switchicon->setScaledContents(true);      //设置label图标适应大小
    connect(ui->label_switchicon, &IconButton::iconClicked,
            [=]()mutable
            {
                static bool windowtop = false;
                if(windowtop == false)
                {
                    this->hide();
                    this->setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);       //如果在一开始就没有设置窗口置顶,那么设置窗口置顶需要先影藏
                    this->show();
                    windowtop = true;       //更改标志位
                }
                else
                {
                    this->hide();
                    this->setWindowFlags(this->windowFlags() & ~Qt::WindowStaysOnTopHint);       //如果在一开始就没有设置窗口置顶,那么设置窗口置顶需要先影藏
                    this->show();
                    windowtop = false;
                }
            });

    //ui->tabWidget相关设置
    ui->tabWidget->setTabText(0, QString("Normal"));        //设置tabwidget提示文字
        ui->tabWidget->setTabText(1, QString("Chat"));
    ui->tabWidget->setTabText(2, QString("Dial"));
    ui->tabWidget->setTabText(3, QString("Draw"));
    ui->tabWidget->setTabText(4, QString("About"));
    ui->tabWidget->setTabShape(QTabWidget::Triangular);     //标签样式
    ui->tabWidget->setTabPosition(QTabWidget::West);        //标签方向

    //***********定时器初始化*************/
    updatebuftimer = new QTimer(this);
    connect(updatebuftimer, &QTimer::timeout,
            [=]()mutable
            {
                if(receivedbuf.isEmpty() == false)      //接收缓冲区有数据
                {
                    QStringList datalist = receivedbuf.toList();
                    QString data = datalist.join("");
                    receivedbuf.clear();
                    ui->textEdit_printdata->moveCursor(QTextCursor::End);
                    ui->textEdit_printdata->insertPlainText(data.toUtf8().data());       //取出栈底数据
                    ui->textEdit_printdata->moveCursor(QTextCursor::End);
                }
                else
                {
                        updatebuftimer->stop();
                }
            });
    updatebuftimer->start(10);
    /************serialport相关设置*****************/
    myserialthread = new QThread(this);     //实例化serialport线程
    myserialwork = new MySerial();      //实例化serialport类
    myserialwork->moveToThread(myserialthread);     //把我的serialport类加到我的从线程中
    myserialthread->start();        //开始我的行程,先执行类的构造处理,只能通过信号与槽启动类中的函数

    //MyWidget signals to  MySerial slots
    connect(this,&MyWidget::openSerialPort, myserialwork, &MySerial::openSerialPort);       //建立打开serialport信号链接
    connect(this,&MyWidget::sendOnceData, myserialwork, &MySerial::sendOnceData);       //建立单次发送信号链接
    connect(this,&MyWidget::closeSerialPort, myserialwork, &MySerial::closeSerialPort);        //建立关闭串口信号链接
    connect(this,&MyWidget::autoSendDate, myserialwork, &MySerial::autoSendDate);     //建立自动发送信号链接
    connect(this,&MyWidget::stopautoSendDate, myserialwork, &MySerial::stopautoSendDate);       //建立停止自动发送信号链接
    connect(this,&MyWidget::setSerialPortName, myserialwork->serialport, &QSerialPort::setPortName);        //建立设置端口名字信号
    connect(this,&MyWidget::setSerialPortBaudRate, myserialwork, &MySerial::setSerialPortBaudRate);        //建立设置端口波特率信号
    connect(this,&MyWidget::setSerialPortParity, myserialwork, &MySerial::setSerialPortParity);        //建立设置端口校验位信号
    connect(this,&MyWidget::setSerialPortDateBits, myserialwork, &MySerial::setSerialPortDateBits);        //建立设置端口数据位信号
    connect(this,&MyWidget::setSerialPortStopBits, myserialwork, &MySerial::setSerialPortStopBits);        //建立设置端口数据位信号
     connect(ui->checkBox_hexreceive,&QCheckBox::clicked, myserialwork, &MySerial::setReceiveMode);     //建立设置端口接收模式信号
     connect(this,&MyWidget::clearsendreceivedtimes, myserialwork, &MySerial::clearsendreceivedtimes);        //建立//清空接收发送次数变量信号
    //测试代码
    //emit openSerialPort();      //发送一个打开serialport信号
    //成功

    //MySerial signals to  MyWidget slots
    connect(myserialwork, &MySerial::updatePrintData, this, &MyWidget::updatePrintData);        //建立接收数据更新到数据显示窗口信号
    connect(myserialwork, &MySerial::serialIsOpen, this, &MyWidget::serialIsOpen);     //建立端口正式打开关闭信号链接
    connect(myserialwork, &MySerial::updatereceivedtimes, this, &MyWidget::updatereceivedtimes);     //建立接收次数信号链接
    connect(myserialwork, &MySerial::updatesendtimes, this, &MyWidget::updatesendtimes);     //建立发送次数信号链接
}

MyWidget::~MyWidget()
{
    delete ui;
}

void MyWidget::setupRealtimeDataDemo(QCustomPlot *customPlot)
{
    customPlot->legend->setVisible(true);
    customPlot->legend->setFont(QFont("Helvetica", 9));
    QPen pen;
    customPlot->addGraph();
    pen.setColor(QColor(Qt::black));
    customPlot->graph()->setPen(pen);
    customPlot->graph()->setName(QString("Qdata"));
    customPlot->graph()->setLineStyle(QCPGraph::lsImpulse);
    customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
    // generate data:
    customPlot->graph()->setData(x, y);
    customPlot->graph()->rescaleAxes(true);
    // zoom out a bit:
    customPlot->yAxis->scaleRange(1.1, customPlot->yAxis->range().center());
    customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
    // set blank axis lines:
    customPlot->xAxis->setTicks(false);
    customPlot->yAxis->setTicks(true);
    customPlot->xAxis->setTickLabels(false);
    customPlot->yAxis->setTickLabels(true);
    customPlot->yAxis->setRange(0, 200);
    // make top right axes clones of bottom left axes:
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
}

void MyWidget::closeEvent(QCloseEvent *event)
{
    if(myserialwork->serialport->isOpen() == true)
    {
        emit closeSerialPort();
    }
    while(myserialwork->serialport->isOpen());
    myserialthread->quit();     //给myserial线程发出退出指令
    while(false == myserialthread->wait());    //等待线程退出
        delete myserialwork;        //释放申请的内存
    QWidget::closeEvent(event);     //退出事件继续向下传递
}

void MyWidget::on_pushButton_switch_clicked()       //关闭和打开串口
{
    if(myserialwork->serialport->isOpen() == true)
    {
        on_checkBox_autosend_clicked(false);        //使用关闭自动发送槽函数
        emit closeSerialPort();     //发送关闭串口信号
        cout << "close";
    }
    else
    {
//        int i = 0;
        //串口初始化代码
        emit setSerialPortName(ui->comboBox_com->currentText());        //发送设置com口信号
        emit setSerialPortBaudRate(ui->comboBox_baud->currentText().toInt());       //设置端口波特率
        emit setSerialPortParity(ui->comboBox_parity->currentIndex() + 1);     //设置serialport端口校验位
        emit setSerialPortDateBits(ui->comboBox_databit->currentIndex());        //设置serialport端口数据位
        emit setSerialPortStopBits(ui->comboBox_stopbit->currentIndex());       //设置serialport停止位
        emit openSerialPort();      //发送打开串口信号
        cout << "open";
    }
}

void MyWidget::serialIsOpen(bool figure)        //端口打开关闭监视标志处理函数
{
    if(figure == true)
    {
        labelmovie->start();        //动画开始
        ui->checkBox_autosend->setChecked(false);       //取消复选框
        ui->pushButton_switch->setText(QString("关闭串口"));
    }
    else
    {
        labelmovie->stop();        //动画停止
        ui->checkBox_autosend->setChecked(false);       //取消复选框
        ui->pushButton_switch->setText(QString("打开串口"));
    }
}

void MyWidget::updatePrintData(QString data)        //显示更新函数
{
    if(data.isEmpty() == false)
    {
        if(ui->tabWidget->currentIndex() == TABWIDGETINDEX::NORMAL)
        {
            receivedbuf.push_back(data);        //压入数据
            if(updatebuftimer->isActive() == false)
            {
                    updatebuftimer->start(10);
        }
        }
        else if(ui->tabWidget->currentIndex() == TABWIDGETINDEX::CHAT)
        {
            ui->checkBox_autosend->setChecked(false);
            on_checkBox_autosend_clicked(false);    //关闭自动发送
            static int minute = 0;
            QTime time;
            time = QTime::currentTime();        //获取系统时间
            QDate date;
            date = QDate::currentDate();        //获取系统日期
            data.replace('\n', "<br/>");        //把字符里面的回车编程html中的换行
            if(minute != time.minute() || whochat == WHOCHAT::ME)       //如果送信时间超过一分钟 或 上次数据是COM,则有时间等内容
            {
                minute = time.minute();
                QString html = ui->textEdit_chatshow->toHtml();     //获取html
                html.replace("</body>",QString("</p><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; color:#23719c;\">COM %1 %2	</span><span style=\" font-size:12pt; color:#760e10;\">  </span></p></body>").arg(date.toString("yyyy/MM/dd")).arg(time.toString("a HH:mm:ss")));
                html.replace("</body>", QString("<ul style=\"-qt-list-indent: 1;\"><li style=\" font-size:12pt; color:#760e10;\" style=\" margin-top:12px;\"><span style=\" font-size:small;\">%1</span></li></ul>").arg(data));
                ui->textEdit_chatshow->setHtml(html);       //首先在body替换数据,达到插入数据的目的
                ui->textEdit_chatshow->moveCursor(QTextCursor::End);
                //ui->textEdit_chatsendbuf->clear();
            }
            else
            {
                QString html = ui->textEdit_chatshow->toHtml();
                //html.replace("</body>",QString("</p><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; color:#21c824;\">Local %1 %2	</span><span style=\" font-size:12pt; color:#760e10;\">  </span></p></body>").arg(date.toString("yyyy/MM/dd")).arg(time.toString("a HH:mm:ss")));
                html.replace("</body>", QString("<ul style=\"-qt-list-indent: 1;\"><li style=\" font-size:9pt; color:#760e10;\" style=\" margin-top:0px;\"><span style=\" font-size:small;\">%1</span></li></ul>").arg(data));
                ui->textEdit_chatshow->setHtml(html);
                ui->textEdit_chatshow->moveCursor(QTextCursor::End);
            }
            whochat = WHOCHAT::COM;     //把发送标志设为COM
        }
        else if(ui->tabWidget->currentIndex() == TABWIDGETINDEX::DRAW)
        {
            ui->checkBox_autosend->setChecked(false);
            on_checkBox_autosend_clicked(false);    //关闭自动发送
            static QString connectdata = QString("");
            //接收的速度不能非人类
            //对数据进行分析处理
            QStringList ls = (connectdata + data).split("##");
            int size = ls.size();       //获取QStringList长度
            connectdata = ls.at(size - 1);      //保存剩下的数据
            int pointvalue = ui->spinBoxpointnumber->value();
            for(int i = 0; i < size - 1; i++)       //遍历列表除了最后一个,把数据压入数组
            {
                if(y.size() < pointvalue)       //已有数据短了,则追加长度
                {
                    x.push_back(x.size());
                    y.push_back(QString(ls.at(i)).toDouble());
                }
                else if(y.size() > pointvalue)      //长了,就POP出数据
                {
                    for(int i = 0; i < x.size() - pointvalue; i ++)
                    {
                        x.pop_back();
                        y.pop_front();
                    }
                    y.pop_front();
                    y.push_back(QString(ls.at(i)).toDouble());
                }
                else        //保持原先的长度就可以了
                {
                    y.pop_front();
                    y.push_back(QString(ls.at(i)).toDouble());
                }
            }
            ui->customplot->graph()->setData(x, y);     //设置数据
            ui->customplot->graph()->rescaleKeyAxis();      //保持数据宽度全屏
            ui->customplot->replot();       //刷新显示
        }
   }
}

void MyWidget::on_pushButton_sendonce_clicked()     //单次发送数据响应函数
{
    if(myserialwork->serialport->isOpen() == true)
    {
        QString data = ui->textEdit_senddata->toPlainText();
        emit sendOnceData(data, ui->checkBox_hexsend->isChecked());     //发射单次发送信号
    }
}

void MyWidget::on_pushButton_savereceivedata_clicked()      //保存接收到的数据
{
    QString path = QFileDialog::getSaveFileName(this, QString("open file"), QString("../"),QString("TXT(*.txt)"));      //获取需要保存文件的路径
    if(path.isEmpty() == false)     //检测路径是否为空
    {
        QFile file(path);       //打开文件
        bool isopen = file.open(QIODevice::WriteOnly);      //用只写模式打开文件
        if(isopen  == true)      //检测文件是否打开成功
        {
            QString str = ui->textEdit_printdata->toPlainText();        //读取接收框的内容
            QStringList ls;
            ls =  str.split('\n');  //适应window换行
            str = ls.join("\r\n");
            //file.write(str.toStdString().data());
            file.write(str.toUtf8().data());      //用本地编码写入数据
            //file.write(str.toUtf8());
            file.close();       //关闭文件
        }
    }
}

void MyWidget::on_checkBox_autosend_clicked(bool checked)       //是否开始自动发送
{
    if(myserialwork->serialport->isOpen() == true)      //如果端口打开,则可以发送信号
    {
        if(checked == true)     //如果复选框被选上,则启动自动发送
        {
            QString data = ui->textEdit_senddata->toPlainText();        //读取文本框的内容
            int time = ui->lineEdit_autosendtime->text().toInt();       //读取自动发送的时间间隔
            emit autoSendDate(data, time, ui->checkBox_hexsend->isChecked());      //发送自动发送信号
        }
        else
        {
            emit stopautoSendDate();        //如果复选框没有选上,则发送停止自动发送信号
        }
    }
    else
    {
        if(myserialwork->autosenddatetimer->isActive() == true)     //保证serialport关闭后,定时器关闭
        {
            emit stopautoSendDate();
        }
    }
}

void MyWidget::on_lineEdit_autosendtime_textChanged(const QString &)
{
    if(myserialwork->serialport->isOpen() == true)      //如果端口打开,则可以发送信号
    {
        if(ui->checkBox_autosend->isChecked() == true)     //如果复选框被选上,则启动自动发送
        {
            QString data = ui->textEdit_senddata->toPlainText();        //读取文本框的内容
            int time = ui->lineEdit_autosendtime->text().toInt();       //读取自动发送的时间间隔
            emit autoSendDate(data, time, ui->checkBox_hexsend->isChecked());      //发送自动发送信号
        }
        else
        {
            emit stopautoSendDate();        //如果复选框没有选上,则发送停止自动发送信号
        }
    }
    else
    {
        if(myserialwork->autosenddatetimer->isActive() == true)     //保证serialport关闭后,定时器关闭
        {
            emit stopautoSendDate();
        }
    }
}

void MyWidget::on_textEdit_senddata_textChanged()
{
    if(myserialwork->serialport->isOpen() == true)      //如果端口打开,则可以发送信号
    {
        if(ui->checkBox_autosend->isChecked() == true)     //如果复选框被选上,则启动自动发送
        {
            QString data = ui->textEdit_senddata->toPlainText();        //读取文本框的内容
            int time = ui->lineEdit_autosendtime->text().toInt();       //读取自动发送的时间间隔
            emit autoSendDate(data, time, ui->checkBox_hexsend->isChecked());      //发送自动发送信号
        }
        else
        {
            emit stopautoSendDate();        //如果复选框没有选上,则发送停止自动发送信号
        }
    }
    else
    {
        if(myserialwork->autosenddatetimer->isActive() == true)     //保证serialport关闭后,定时器关闭
        {
            emit stopautoSendDate();
        }
    }
}

void MyWidget::on_checkBox_hexsend_clicked()
{
    if(myserialwork->serialport->isOpen() == true)      //如果端口打开,则可以发送信号
    {
        if(ui->checkBox_autosend->isChecked() == true)     //如果自动发送复选框被选上,则启动自动发送
        {
            QString data = ui->textEdit_senddata->toPlainText();        //读取文本框的内容
            int time = ui->lineEdit_autosendtime->text().toInt();       //读取自动发送的时间间隔
            emit autoSendDate(data, time, ui->checkBox_hexsend->isChecked());      //发送自动发送信号
        }
        else
        {
            emit stopautoSendDate();        //如果复选框没有选上,则发送停止自动发送信号
        }
    }
    else
    {
        if(myserialwork->autosenddatetimer->isActive() == true)     //保证serialport关闭后,定时器关闭
        {
            emit stopautoSendDate();
        }
    }
}


//dial发送数据
void MyWidget::on_dial_valueChanged(int value)      //每次dial数值改变,发送相应的Ascall
{
    char i = char(value);
    ui->checkBox_autosend->setChecked(false);
    on_checkBox_autosend_clicked(false);    //关闭自动发送
    QString data = QString("%1").arg(i);
    emit sendOnceData(data, false);     //发射单次发送信号
}

void MyWidget::on_pushButton_clear_clicked()        //发送接收次数清零
{
    ui->lineEditRX->setText(QString("0"));
    ui->lineEdit_TX->setText(QString("0"));
    emit clearsendreceivedtimes();
}

void MyWidget::updatereceivedtimes(qint64 times)        //更新接收次数多少
{
    ui->lineEditRX->setText(QString("%1").arg(times));
}


void MyWidget::updatesendtimes(qint64 times)        //更新发送数据的多少
{
    ui->lineEdit_TX->setText(QString("%1").arg(times));
}


//画图发送函数
void MyWidget::on_tab_3pushbuttonsend_clicked()// tab_3页按钮发送数据
{
    ui->checkBox_autosend->setChecked(false);
    on_checkBox_autosend_clicked(false);    //关闭自动发送
    if(myserialwork->serialport->isOpen() == true)
    {
        QString data = ui->tab_3texteditsenddata->text();
        emit sendOnceData(data, false);     //发射单次发送信号
    }
}


//chat页,发送数据
void MyWidget::on_pushButton_sendchat_clicked()
{
    QTime time;
    time = QTime::currentTime();        //获取当地时间
    static int minute = time.minute();
    QDate date;
    date = QDate::currentDate();        //获取当地日期
    QString data = ui->textEdit_chatsendbuf->toPlainText();     //获取文本框的数据
    ui->checkBox_autosend->setChecked(false);
    on_checkBox_autosend_clicked(false);    //关闭自动发送
    if(myserialwork->serialport->isOpen() == true)
    {
        emit sendOnceData(data, ui->checkBox_hexsend->isChecked());     //发射单次发送信号
    }
    data.replace('\n', "<br/>");
    if(data.isEmpty() == false)
    {
//        ui->textEdit_chatshow->moveCursor(QTextCursor::End);
//        ui->textEdit_chatshow->insertHtml(QString("</p><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; color:#21c824;\">Local %1 %2	</span><span style=\" font-size:12pt; color:#760e10;\">  </span></p>").arg(date.toString("yyyy/MM/dd")).arg(time.toString("a HH:mm:ss")));
//        ui->textEdit_chatshow->moveCursor(QTextCursor::End);
//        ui->textEdit_chatshow->insertHtml(QString("<ul style=\"-qt-list-indent: 1;\"><li style=\" font-size:12pt; color:#760e10;\" style=\" margin-top:12px;\"><span style=\" font-size:small;\">%1</span></li></ul>").arg(data));
//        ui->textEdit_chatshow->moveCursor(QTextCursor::End);
//        ui->textEdit_chatsendbuf->clear();
        if(minute != time.minute() || whochat == WHOCHAT::COM)
        {
            minute = time.minute();
            QString html = ui->textEdit_chatshow->toHtml();
            html.replace("</body>",QString("</p><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; color:#21c824;\">Local %1 %2	</span><span style=\" font-size:12pt; color:#760e10;\">  </span></p></body>").arg(date.toString("yyyy/MM/dd")).arg(time.toString("a HH:mm:ss")));
            html.replace("</body>", QString("<ul style=\"-qt-list-indent: 1;\"><li style=\" font-size:12pt; color:#760e10;\" style=\" margin-top:12px;\"><span style=\" font-size:small;\">%1</span></li></ul>").arg(data));
            ui->textEdit_chatshow->setHtml(html);
            ui->textEdit_chatshow->moveCursor(QTextCursor::End);
            ui->textEdit_chatsendbuf->clear();
        }
        else
        {
            QString html = ui->textEdit_chatshow->toHtml();
            //html.replace("</body>",QString("</p><p style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; color:#21c824;\">Local %1 %2	</span><span style=\" font-size:12pt; color:#760e10;\">  </span></p></body>").arg(date.toString("yyyy/MM/dd")).arg(time.toString("a HH:mm:ss")));
            html.replace("</body>", QString("<ul style=\"-qt-list-indent: 1;\"><li style=\" font-size:9pt; color:#760e10;\" style=\" margin-top:0px;\"><span style=\" font-size:small;\">%1</span></li></ul>").arg(data));
            ui->textEdit_chatshow->setHtml(html);
            ui->textEdit_chatshow->moveCursor(QTextCursor::End);
            ui->textEdit_chatsendbuf->clear();
        }
        whochat = WHOCHAT::ME;
    }
}
