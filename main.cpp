#include "mainwindow.h"

#include <QApplication>

#include <QDebug>

#include "modbustcpclient.h"

int main(int argc, char* argv[])
{
    qDebug() << " ";
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    ModbusTcpClient client;
    qDebug() << "modbustcp" << client.connect();

    client.write(ModbusTcpClient::WriteSingleRegister, 0x00, uint16_t(3));
    client.write(ModbusTcpClient::WriteMultipleRegisters, 0x02, float(25.5), float(29.2), float(9.98));

    return a.exec();
}
