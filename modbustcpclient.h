#ifndef MODBUSTCPCLIENT_H
#define MODBUSTCPCLIENT_H

#include "modbusformat.h"
#include <QTcpSocket>

//485
class ModbusTcpClient {

private:
    ModBusFormat modbusFmt;

    uint16_t timeout = 2000; ///ms

    uint16_t transactionId = 0;
    uint16_t protocol = 0;
    uint8_t unitId = 1;

    uint32_t ipv4 = ((127 << 24) | 1);
    uint16_t port = 502;

    QTcpSocket socket;
    QByteArray resBytes;

public:
    enum OPT {
        ReadCoils = 0x01,
        ReadDiscreteInputs = 0x02,
        ReadHoldingRegisters = 0x03,
        ReadInputRegisters = 0x04,
        WriteSingleCoil = 0x05,
        WriteSingleRegister = 0x06,
        WriteMultipleCoils = 0x0F,
        WriteMultipleRegisters = 0x10,
        MaskWriteRegister = 0x16,
        //        ReadWriteMultipleRegisters = 0x17,
    };

    enum RET_CODE {
        OK_OPT,
        ERR_OPT_CODE,
        ERR_NUM_OF_PARAMS,
        ERR_NETWORK,
    };

    ModbusTcpClient();

    bool connect(int ipv4, int port, uint16_t waitTime = 3000);
    bool connect(uint16_t waitTime = 3000);

    void setFormat(const ModBusFormat& fmt)
    {
        modbusFmt = fmt;
    }

    const ModBusFormat& format() const
    {
        return modbusFmt;
    }

    void setEndian(int ed)
    {
        modbusFmt.setServerEndian(ed);
    }

    void setUnitId(uint8_t uid)
    {
        unitId = uid;
    }

    void setTransactionId(uint16_t tid)
    {
        transactionId = tid;
    }

    //须保证每个变量字节为 8字节以下
    template <typename... Args>
    int write(OPT opt, uint16_t addr, const Args&... args)
    {
        //获取参数长度
        uint16_t count_params = sizeof...(args) + 2;

        if (0x01 > count_params || 0x07D0 < count_params)
            return ERR_NUM_OF_PARAMS; //参数数量错误

        char* data = new char[count_params * 8];

        uint16_t index = addModbusTcpHead(data);

        //功能码
        index += copyToData(data + index, uint8_t(opt));

        //地址
        index += copyToData(data + index, addr);

        switch (opt) {
        case WriteMultipleCoils:
        case WriteMultipleRegisters: {

            index += 3;
        } break;
        default:
            break;
        }

        int ret = write(data, opt, index, args...);

        //网络故障
        if (ret == ERR_NETWORK) {
            return ERR_NETWORK;
        }

        //等待恢复
        socket.waitForReadyRead();
        resBytes = socket.readAll();

        delete[] data;
        return ret;
    }

    QByteArray getResult()
    {
        return resBytes;
    }

private:
    template <typename T, typename... Args>
    int write(char* data, OPT opt, uint16_t index, T first, const Args&... args)
    {
        index += copyToData(data + index, first);
        return write(data, opt, index, args...);
    }

    template <typename T>
    int write(char* data, OPT opt, uint16_t index, T end)
    {
        index += copyToData(data + index, end);

        //数据长度
        copyToData(data + 4, uint16_t(index - 6));

        switch (opt) {
        case WriteMultipleCoils:
        case WriteMultipleRegisters: {
            uint8_t bcount = (index - 13);
            uint16_t wcount = bcount / 2;
            copyToData(data + 10, wcount);
            copyToData(data + 12, bcount);
        } break;
        default:
            break;
        }

        //Send Data;
        socket.write(data, index);
        return socket.waitForBytesWritten() ? ERR_NETWORK : OK_OPT;
    }

    //data
    template <typename T>
    int copyToData(char* data, T value)
    {
        int n = sizeof(value);
        T tmp = modbusFmt.toServer(value);
        char* bytes = reinterpret_cast<char*>(&tmp);
        int i = 0;
        while (i < n) {
            data[i] = bytes[i];
            ++i;
        }
        return i;
    }

    int addModbusTcpHead(char* data)
    {
        int index = 0;
        //事务id
        index += copyToData(data + index, transactionId);

        //事务号自动++
        ++transactionId;

        //协议标识
        index += copyToData(data + index, protocol);

        //空两个字节
        index += 2;

        index += copyToData(data + index, unitId);

        return index;
    }

private:
};

#endif // MODBUSTCPCLIENT_H
