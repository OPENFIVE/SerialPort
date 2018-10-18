#include "myserial.h"
#include <QtSerialPort/QSerialPort>
#include  <QSerialPortInfo>
#include <QDebug>
#include <QThread>
#define cout  qDebug() << __LINE__ << ":"
MySerial::MySerial(QObject *parent) : QObject(parent)
{
    //变量初始化
    isreceivehex = false;
    sendtimes = 0;
    receivetimes = 0;
    //初始化定时器
    autosenddata = QByteArray("");
    autosenddatetimer =new QTimer(this);
    connect(autosenddatetimer, &QTimer::timeout,        //定时发送定时器,定时发送数据
            [=]()mutable
    {
        if(serialport->isOpen() == true)
        {
            if(autosenddata.isEmpty() == false)
            {
                sendtimes += serialport->write(autosenddata.data());
                emit updatesendtimes(sendtimes);
            }
            else
            {
                autosenddatetimer->stop();
            }
        }
        else
        {
            autosenddatetimer->stop();
        }
    });
    //初始化串口
    serialport = new QSerialPort(this);
    serialport->setPortName(QString("COM2"));       //设置初始串口号
    serialport->setBaudRate(QSerialPort::Baud9600);         //设置初始波特率
    serialport->setParity(QSerialPort::NoParity);           //设置没有校验位
    serialport->setDataBits(QSerialPort::Data8);            //设置数据位为8位
    serialport->setStopBits(QSerialPort::TwoStop);      //设置停止位为1位
    //serialport->open(QIODevice::ReadWrite);         //设置buf为读写模式
    //void QSerialPort::setPort(const QSerialPortInfo &serialPortInfo);
    //debug
    //

    connect(serialport, &QSerialPort::readyRead,        //当收到字符时候,启动处理接收数据函数
            [=]()mutable        //lambda表达式
            {
                        QByteArray data = serialport->readAll();      //读取全部字符串
                        receivetimes += data.length();
                        emit updatereceivedtimes(receivetimes);
                        if(isreceivehex  == true)        //把串口读出来的数据刷到我的显示textEdit上
                        {
                            QString temp(data.toHex().toUpper());
                            int len = temp.length()/2;
                            for(int i=1;i<len + 1;i++)
                            {
                                temp.insert(2*i+i - 1, " ");
                            }
                            emit updatePrintData(temp);     //发送信号,把收到的数据(hex)送到主线程,供给相应的函数进行处理
                        }
                        else
                        {
                            emit updatePrintData(QString(data));     //发送信号,把收到的数据(hex)送到主线程,供给相应的函数进行处理
                        }
            });
}


void MySerial::openSerialPort(void)
{
    //测试设置QSerialPortInfo
    if(serialport->isOpen() == false)
    {
        serialport->open(QIODevice::ReadWrite);
        cout << "try open";
        if(serialport->isOpen() == true)      //等待串口打开
        {
            emit serialIsOpen(true);
            cout << "emit open";
        }
        else
        {
            emit serialIsOpen(false);
            cout << "emit CLOSE";
        }
        //测试代码:端口设置
//        serialport->setPortName(QString("COM4"));
//        serialport->close();
//        cout << serialport->portName();
//        serialport->open(QIODevice::ReadWrite);
        //端口名字设置后,需要重启才能生效

        //测试代码:自动发送
//        autoSendDate(QString("auto"), 1);
//        stopautoSendDate();
        //实验成功
    }
}

void MySerial::closeSerialPort(void)        //关闭serialport
{
    if(autosenddatetimer->isActive() == true)       //如果自动发送定时器还在活动关闭定时器
    {
        autosenddatetimer->stop();
    }
    if(serialport->isOpen() == true)        //如果串口处于打开状态,关闭串口
    {
        serialport->close();
        cout << "try close";
    }
    if(serialport->isOpen() == false)
    {
        emit serialIsOpen(false);
    }
}

void MySerial::setReceiveMode(bool ishex)       //设置是否以hex模式接收数据
{
    isreceivehex = ishex;
}

void MySerial::sendOnceData(QString oncedate, bool hexsend)       //发送一次数据
{
    if(serialport->isOpen() == true)        //串口打开,才发送数据
    {
        if(hexsend == true)
        {
            bool ok;
            QByteArray ret;
            oncedate = oncedate.simplified();
            QStringList sl = oncedate.split(" ");
            foreach (QString s, sl) {
                if(s.isEmpty() == false)
                {
                    char c = s.toInt(&ok,16)&0xFF;
                    if(ok){
                        ret.append(c);
                    }
                }
            }
           sendtimes += serialport->write(ret.data());        //Hex发送
           emit updatesendtimes(sendtimes);
        }
        else
        {
            QStringList ls;
            ls =  oncedate.split('\n');  //适应window换行
            oncedate = ls.join("\r\n");
            sendtimes += serialport->write(oncedate.toUtf8().data());        //正常发送
            emit updatesendtimes(sendtimes);
        }
    }
}

void MySerial::autoSendDate(QString data, int time, bool hexsend)     //自动循环发送
{
    //autosenddata = data.toUtf8();        //传递需要发送的字符串
    if(hexsend == false)
    {
        QStringList ls;
        ls =  data.split('\n');  //适应window换行
        data = ls.join("\r\n");
        autosenddata =data.toUtf8();
    }
    else
    {
        bool ok;
        QByteArray ret;
        data = data.simplified();
        QStringList sl = data.split(" ");
        foreach (QString s, sl) {
            if(s.isEmpty() == false)
            {
                char c = s.toInt(&ok,16)&0xFF;
                if(ok){
                    ret.append(c);
                }
            }
        }
        autosenddata = ret;
    }
    autosenddatetimer->start(time);     //启动发送定时器
}

void MySerial::stopautoSendDate(void)       //停止自动发送
{
    autosenddata.clear();
    autosenddatetimer->stop();      //只需要停止自动发送定时器即可
}

void MySerial::clearsendreceivedtimes(void)     //清空接收发送次数变量
{
    sendtimes = 0;
    receivetimes = 0;
}

void MySerial::setSerialPortBaudRate(qint32 baud)     //设置serialport端口波特率信号
{
    switch (baud) {
    case QSerialPort::Baud1200:
        serialport->setBaudRate(QSerialPort::Baud1200);
        break;
    case QSerialPort::Baud2400:
        serialport->setBaudRate(QSerialPort::Baud2400);
        break;
    case QSerialPort::Baud4800:
        serialport->setBaudRate(QSerialPort::Baud4800);
        break;
    case QSerialPort::Baud9600:
        serialport->setBaudRate(QSerialPort::Baud9600);
        break;
    case QSerialPort::Baud19200:
        serialport->setBaudRate(QSerialPort::Baud19200);
        break;
    default:
        break;
    }
}

void MySerial::setSerialPortParity(qint32 parity)     //设置serialport端口校验位
{
    switch (parity) {
    case 1:
        serialport->setParity(QSerialPort::NoParity);
        break;
    case 2:
        serialport->setParity(QSerialPort::EvenParity);
        break;
    case 3:
        serialport->setParity(QSerialPort::OddParity);
        break;
    case 4:
        serialport->setBaudRate(QSerialPort::SpaceParity);
        break;
    case 5:
        serialport->setBaudRate(QSerialPort::MarkParity);
        break;
    default:
        break;
    }
}
void MySerial::setSerialPortDateBits(qint32 bits)       //设置串口数据位
{
    switch (bits) {
    case 0:
        serialport->setDataBits(QSerialPort::Data5);
        break;
    case 1:
        serialport->setDataBits(QSerialPort::Data6);
        break;
    case 2:
        serialport->setDataBits(QSerialPort::Data7);
        break;
    case 3:
        serialport->setDataBits(QSerialPort::Data8);
        break;
    default:
        break;
    }
}

void MySerial::setSerialPortStopBits(qint32 bits)       //设置串口停止位
{
    switch (bits) {
    case 0:
        serialport->setStopBits(QSerialPort::OneStop);
        break;
    case 1:
        serialport->setStopBits(QSerialPort::OneAndHalfStop);
        break;
    case 2:
        serialport->setStopBits(QSerialPort::TwoStop);
        break;
    default:
        break;
    }
}

