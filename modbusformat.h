#ifndef MODBUSFORMAT_H
#define MODBUSFORMAT_H

#include <type_traits>

//默认本机为大端或者小端 ，本机如果为其他字节序不支持转换

class ModBusFormat {

private:
    union {
        short value = 0x0001;
        bool isLittleEndian : 1;
    };

    enum Endian {
        Endian_ABCD,
        Endian_CDAB,
        Endian_DCBA,
        Endian_BADC,
    };

    int serverEndian = Endian_CDAB;

public:
    ModBusFormat() = default;

    void setServerEndian(int e = Endian_CDAB)
    {
        serverEndian = e;
    }

    int getServerEndian()
    {
        return serverEndian;
    }

    int getClientEndian()
    {
        return isLittleEndian ? Endian_ABCD : Endian_DCBA;
    }

    template <typename T>
    T toClient(T v) const
    {
        switch (serverEndian) {
        case Endian_ABCD:
            return fromABCD(v);
            break;
        case Endian_CDAB:
            return fromCDAB(v);
            break;
        case Endian_DCBA:
            return fromDCBA(v);
            break;
        case Endian_BADC:
            return fromBADC(v);
            break;

        default:
            return v;
        }
    }

    template <typename T>
    T toServer(T v) const
    {
        switch (serverEndian) {
        case Endian_ABCD:
            return toABCD(v);
            break;
        case Endian_CDAB:
            return toCDAB(v);
            break;
        case Endian_DCBA:
            return toDCBA(v);
            break;
        case Endian_BADC:
            return toBADC(v);
            break;

        default:
            return v;
        }
    }

    template <typename T>
    T toABCD(T v) const // to ABCD
    {
        if (isLittleEndian) { //DCBA => ABCD
            return swapByte(v);
        } else { // ABCD => ABCD
            return v;
        }
    }

    template <typename T>
    T toCDAB(T v) const
    {
        if (isLittleEndian) {
            return swapByte(swapByte(v), 2);
        } else {
            return swapByte(v, 2);
        }
    }

    template <typename T>
    T toDCBA(T v) const
    {
        if (isLittleEndian) {
            return v;
        } else {
            return swapByte(v);
        }
    }

    template <typename T, int N = sizeof(T)>
    T toBADC(T v) const
    {
        if (N <= 2) {
            return isLittleEndian ? swapByte(v) : swapByte(v);
        }

        if (isLittleEndian) { // DCBA ->BADC
            return swapByte(v, 2);
        } else { //ABCD => BADC
            return swapByte(swapByte(v), 2);
        }
    }

    template <typename T>
    T fromDCBA(T v) const
    {
        if (isLittleEndian) { // DCBA => DCBA
            return v;
        } else { //DCBA => ABCD
            return swapByte(v);
        }
    }

    template <typename T>
    T fromBADC(T v) const
    {
        if (isLittleEndian) { // BADC => DCBA
            return swapByte(v, 2);
        } else { //DCBA => ABCD
            return swapByte(swapByte(v, 2));
        }
    }

    template <typename T>
    T fromCDAB(T v) const
    {
        if (isLittleEndian) { // CDAB => DCBA
            return swapByte(swapByte(v, 2));
        } else { //CDAB => ABCD
            return swapByte(v, 2);
        }
    }

    template <typename T>
    T fromABCD(T v) const
    {
        if (isLittleEndian) { // ABCD => DCBA
            return swapByte(v);
        } else { //ABCD => ABCD
            return v;
        }
    }

    template <typename T, int N = sizeof(T)>
    T swapByte(T v, int n = 1) const // n个字节为一组， 前n个字节和后面n个字节交换
    {
        if (N <= n)
            return v;

        int end = N / n - 1;
        switch (n) {
        case 1: {

            char* byte = reinterpret_cast<char*>(&v);
            int start = 0;
            while (start < end) {
                std::swap(byte[start++], byte[end--]);
            }
        } break;
        case 2: {
            short* byte = reinterpret_cast<short*>(&v);
            int start = 0;
            while (start < end) {
                std::swap(byte[start++], byte[end--]);
            }
        } break;
        case 4: {
            int* byte = reinterpret_cast<int*>(&v);
            int start = 0;
            while (start < end) {
                std::swap(byte[start++], byte[end--]);
            }
        } break;
        default:
            break;
        }
        return v;
    }
};

#endif // MODBUSFORMAT_H
