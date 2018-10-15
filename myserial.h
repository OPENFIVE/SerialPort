#ifndef MYSERIAL_H
#define MYSERIAL_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
class MySerial : public QObject
{
    Q_OBJECT
public:
    explicit MySerial(QObject *parent = 0);

signals:
    void updatePrintData(QString);      //更新显示信号
    void serialIsOpen(bool);        //串口是否打开信号
    void updatereceivedtimes(qint64 );      //更新接收次数信号
    void updatesendtimes(qint64 );      //更新发送次数信号

public slots:
    void openSerialPort(void);      //打开串口
    void setSerialPortBaudRate(qint32);     //设置serialport端口波特率信号
    void setSerialPortParity(qint32);     //设置serialport端口校验位
    void setSerialPortDateBits(qint32);     //设置serialport端口数据位
    void setSerialPortStopBits(qint32);     //设置serialport端口停止位
    void closeSerialPort(void);     //关闭串口
    void sendOnceData(QString oncedate, bool hexsend);        //发送一次数据
    void autoSendDate(QString data, int time, bool hexsend);      //自动发送数据
    void setReceiveMode(bool);      //设置接收的模式
    void stopautoSendDate(void);        //停止自动发送数据
    void clearsendreceivedtimes(void);      //清空接收发送次数变量
private:
    QSerialPort *serialport;        //串口类
    QTimer *autosenddatetimer;      //自动发送定时器
    QByteArray autosenddata;       //自动发送所发送的字符串

private:
    bool isreceivehex;
    qint64 sendtimes;       //发送次数
    qint64 receivetimes;        //接收次数

protected:

friend class MyWidget;
};

#endif // MYSERIAL_H
