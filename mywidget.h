#ifndef MYWIDGET_H
#define MYWIDGET_H
#include <QWidget>
#include <QThread>
#include <myserial.h>
#include <QtSerialPort/QSerialPort>
#include "qcustomplot.h"
#include <QVector>
enum  WHOCHAT {ME = true, COM = false};
enum TABWIDGETINDEX {NORMAL = 0, CHAT = 1, DIAL = 2, DRAW = 3, ABOUT = 4 };
namespace Ui {
class MyWidget;
}

class MyWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MyWidget(QWidget *parent = 0);
    ~MyWidget();
    void setupRealtimeDataDemo(QCustomPlot *customPlot);

signals:
    void openSerialPort(void);      //打开串口信号
    void sendOnceData(QString oncedate, bool hexsend);        //发送一次数据信号
    void setSerialPortName(QString);        //设置serialport端口名字信号
    void setSerialPortBaudRate(qint32);     //设置serialport端口波特率信号
    void setSerialPortParity(qint32);     //设置serialport端口校验位
    void setSerialPortDateBits(qint32);     //设置serialport端口数据位
    void setSerialPortStopBits(qint32);     //设置serialport端口数据位
    void closeSerialPort(void);     //关闭serialport
    void autoSendDate(QString data, int time, bool hexsend);      //自动发送数据
    void setReceiveMode(bool);      //设置接收的模式
    void stopautoSendDate(void);        //停止自动发送数据
    void clearsendreceivedtimes(void);      //清空接收发送次数变量

public slots:
    void updatePrintData(QString );     //更新数据显示窗口槽函数
    void serialIsOpen(bool);        //端口打开关闭监视标志处理函数
    void updatereceivedtimes(qint64 );      // //更新接收次数信号
    void updatesendtimes(qint64 );      //更新发送次数信号

protected:
    void closeEvent(QCloseEvent *event);        //重写关闭程序事件

private slots:
    void on_pushButton_sendonce_clicked();      //发送一次数据按键事件


    void on_pushButton_savereceivedata_clicked();

    void on_checkBox_autosend_clicked(bool checked);

    void on_lineEdit_autosendtime_textChanged(const QString &arg1);

    void on_textEdit_senddata_textChanged();

    void on_dial_valueChanged(int value);

    void on_pushButton_switch_clicked();

    void on_checkBox_hexsend_clicked();

    void on_pushButton_clear_clicked();

    void on_tab_3pushbuttonsend_clicked();

    void on_pushButton_sendchat_clicked();

private:
    Ui::MyWidget *ui;
    QThread *myserialthread;        //串口工作线程
    MySerial *myserialwork;     //串口类
    QVector<double>x,y;
    QMovie *labelmovie;
    QVector<QString>receivedbuf;
    QTimer *updatebuftimer;
    bool whochat;
};

#endif // MYWIDGET_H
