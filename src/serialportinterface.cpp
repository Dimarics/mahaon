#include "serialportinterface.h"
#include <QSerialPortInfo>

SerialPortInterface::SerialPortInterface()
    : m_serialPort(new QSerialPort(this))
    , m_connectTimer(new QTimer(this))
    , m_timeout(new QTimer(this))
{
    /*
    uint8_t ar[] {
        1, 2, 3, 4, 5, 6, 7, 8
    };
    uint8_t *a = ar + 2;
    qDebug() << a[1];
    */

    m_serialPort->setBaudRate(9600);
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPortInterface::readPort);
    connect(m_serialPort, &QSerialPort::errorOccurred, this, [this](QSerialPort::SerialPortError error) {
        switch (error) {
        case QSerialPort::DeviceNotFoundError:
        case QSerialPort::PermissionError:
        case QSerialPort::ResourceError:
            disconnectDevice();
            break;
        default: break;
        }
    });
    //
    connect(this, &SerialPortInterface::messageAccepted, this, [this] {
        //qDebug() << m_queue;
        if (!m_queue.isEmpty()) {
            m_queue.removeFirst();
            m_timeout->stop();
        }
        if (!m_queue.isEmpty()) {
            m_serialPort->write(m_queue.first());
            m_timeout->start();
        }
        //qDebug() << m_queue;
    });
    //
    m_timeout->setInterval(500);
    connect(m_timeout, &QTimer::timeout, this, [this] {
        qDebug() << "timeout error";
        m_queue.clear();
        m_timeout->stop();
    });

    m_connectTimer->setInterval(1000);
    connect(m_connectTimer, &QTimer::timeout, this, [this] {
        if (!m_serialPort->isOpen()) {
            QList<QSerialPortInfo> port_list = QSerialPortInfo::availablePorts();
            for (QSerialPortInfo &serialPortInfo : port_list) {
                // qDebug() << "check port:" << serialPortInfo.portName();
                // qDebug() << "vendor identifier:" << serialPortInfo.vendorIdentifier();
                // qDebug() << "product identifier:" << serialPortInfo.productIdentifier();
                // if (serialPortInfo.vendorIdentifier() == 6790 &&
                //         serialPortInfo.productIdentifier() == 29987) // arduino uno
                if (serialPortInfo.vendorIdentifier() == 6790 &&
                        serialPortInfo.productIdentifier() == 29987) // LoRa Ebyte E22-400T22U
                {
                    qDebug() << "set port...";
                    m_serialPort->setPort(serialPortInfo);
                    qDebug() << "success!";
                    if (m_serialPort->open(QIODevice::ReadWrite)) {
                        //m_serialPort->setDataTerminalReady(false);
                        qDebug() << "connected:" << m_serialPort->portName();
                        m_connectTimer->stop();
                        break;
                    } else {
                        qDebug() << "connection error";
                    }
                }
            }
        }
    });
    m_connectTimer->start();
    /*
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this] {
        if (m_serialPort->isOpen()) {
            uint16_t stick_1_x = 621;
            uint16_t stick_1_y = 722;
            uint16_t stick_2_x = 823;
            uint16_t stick_2_y = 924;
            uint8_t data[8] {
                uint8_t(stick_1_x >> 8), uint8_t(stick_1_x),
                uint8_t(stick_1_y >> 8), uint8_t(stick_1_y),
                uint8_t(stick_2_x >> 8), uint8_t(stick_2_x),
                uint8_t(stick_2_y >> 8), uint8_t(stick_2_y),
            };
            sendPacket(MSG_ID_SET_MOVE_PARAMS, data, 8);
        }
    });
    timer->start();
    */
}

void SerialPortInterface::setMoveParams(uint16_t stick_1_x, uint16_t stick_1_y, uint16_t stick_2_x, uint16_t stick_2_y)
{
    // qDebug() << stick_1_x << stick_1_y << stick_2_x << stick_2_y;
    // uint8_t data[8] {
    //     uint8_t(stick_1_x >> 8), uint8_t(stick_1_x),
    //     uint8_t(stick_1_y >> 8), uint8_t(stick_1_y),
    //     uint8_t(stick_2_x >> 8), uint8_t(stick_2_x),
    //     uint8_t(stick_2_y >> 8), uint8_t(stick_2_y),
    // };
    // sendPacket(MSG_ID_SET_MOVE_PARAMS, data, 8);
}

void SerialPortInterface::setPortName(const QString &portName) {
    if (portName == m_serialPort->portName()) return;
    /*
    for (const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts()) {
        if (serialPortInfo.portName() == portName) {
            m_serialPort->setPort(serialPortInfo);
            m_serialPort->open(QIODeviceBase::ReadWrite);
        }
    }
    */
    if (m_serialPort->isOpen()) m_serialPort->close();
    m_serialPort->setPortName(portName);
    emit portNameChanged();
}

void SerialPortInterface::connectDevice()
{
    if (m_serialPort->isOpen()) return;
    if (m_serialPort->open(QIODeviceBase::ReadWrite)) {
        //m_serialPort->setDataTerminalReady(true);
    } else {
    }
}

void SerialPortInterface::disconnectDevice()
{
    if (m_serialPort->isOpen()) {
        m_serialPort->close();
        m_connectTimer->start();
    }
}

void SerialPortInterface::send(const QByteArray &message)
{
    m_serialPort->write(message);
}

void SerialPortInterface::sendPacket(uint8_t function_code)
{
    QByteArray packet(5, 0);
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = 0;
    packet[3] = function_code;
    packet[4] = function_code;
    send(packet);
}

void SerialPortInterface::sendPacket(uint8_t msg_id, uint8_t *data, uint8_t size)
{
    QByteArray packet(5 + size, 0);
    packet[0] = 0xFF;
    packet[1] = 0xFF;
    packet[2] = msg_id;
    packet[3] = size;
    for (int i = 0; i < size; ++i) packet[i+4] += data[i];
    uint8_t crc = msg_id + size;
    for (int i = 0; i < size; ++i) crc += data[i];
    packet[size + 4] = crc;
    send(packet);
}

void SerialPortInterface::readPort() {
    // минимальный размер пакета: 4(header) + 1(crc) = 5

    // qDebug() << QString(m_serialPort->readAll());
    // return;

#pragma pack(push, 1)
    struct Header {
        quint16 startBytes = 0;
        quint8 function_code = 0;
        quint8 size = 0;
    };
#pragma pack(pop)

    // поиск заголовка
    Header header;
    serialPort()->peek(reinterpret_cast<char*>(&header), sizeof(Header));
    while (header.startBytes != 0xFFFF && serialPort()->bytesAvailable() >= 5) {
        serialPort()->read(1);
        serialPort()->peek(reinterpret_cast<char*>(&header), sizeof(Header));
    }
    if (header.startBytes != 0xFFFF || serialPort()->bytesAvailable() - 5 < header.size) return;
    serialPort()->read(sizeof(Header));

    uint8_t *data = new uint8_t[header.size];
    uint8_t crc = header.function_code + header.size;
    if (header.size) {
        serialPort()->read(reinterpret_cast<char*>(data), header.size);
        for (int i = 0; i < header.size; ++i) crc += data[i]; // подсчёт контрольной суммы
    }

    if (crc != getByte()) {
        qDebug() << "ошибка контрольной суммы";
        return;
    }

    // обработка данных
    switch(header.function_code) {
    case MSG_ID_SET_MOVE_PARAMS: {
        uint16_t stick_1_x = uint16_t(data[0]) << 8 | data[1];
        uint16_t stick_1_y = uint16_t(data[2]) << 8 | data[3];
        uint16_t stick_2_x = uint16_t(data[4]) << 8 | data[5];
        uint16_t stick_2_y = uint16_t(data[6]) << 8 | data[7];
        qDebug() << "=========================";
        qDebug() << "stick_1_x: " << stick_1_x;
        qDebug() << "stick_1_y: " << stick_1_y;
        qDebug() << "stick_2_x: " << stick_2_x;
        qDebug() << "stick_2_y: " << stick_2_y;
    }
        break;
    case MSG_ID_YAW_CHANGED: {
        float yaw;
        std::memcpy(&yaw, data, 4);
        qDebug() << "yaw:" << yaw;
        emit yawChanged(yaw);
    }
        break;
    case MSG_ID_GLOBAL_POSITION_CHANGED: {
        float latitude;
        float longitude;
        uint16_t altitude;
        bool valid = data[10];
        std::memcpy(&latitude, &data[0], 4);
        std::memcpy(&longitude, &data[4], 4);
        std::memcpy(&altitude, &data[8], 2);
        qDebug() << "latitude:" << latitude << "longitude:" << longitude << "altitude:" << altitude << "valid:" << valid;
        emit globalPositionChanged(latitude, longitude, altitude);
    }
        break;
    default:
        qDebug() << "неизвестный код функции";
        break;
    }

    delete[] data;

    // поиск заголовка
// #pragma pack(push, 1)
//     struct Header {
//         quint16 startBytes = 0;
//         quint8 function_code = 0;
//         quint8 size = 0;
//     };
// #pragma pack(pop)

//     // поиск заголовка
//     Header header;
//     serialPort()->peek(reinterpret_cast<char*>(&header), sizeof(Header));
//     while (header.startBytes != 0xFFFF && serialPort()->bytesAvailable() >= 5) {
//         serialPort()->read(1);
//         serialPort()->peek(reinterpret_cast<char*>(&header), sizeof(Header));
//     }
//     if (header.startBytes != 0xFFFF || serialPort()->bytesAvailable() - 5 < header.size) return;

//     uint8_t *packet = new uint8_t[header.size + 5];
//     serialPort()->read(reinterpret_cast<char*>(&packet), header.size + 5);
//     uint8_t *data = &packet[4];

//     delete[] packet;
    readPort();
}
