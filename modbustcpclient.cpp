#include "modbustcpclient.h"

#include <QHostAddress>

ModbusTcpClient::ModbusTcpClient()
{
}

bool ModbusTcpClient::connect(int ip, int pt, uint16_t waitTime)
{
    ipv4 = ip;
    port = pt;
    return connect(waitTime);
}

bool ModbusTcpClient::connect(uint16_t waitTime)
{
    socket.connectToHost(QHostAddress(ipv4), port);
    return socket.waitForConnected(waitTime);
}
