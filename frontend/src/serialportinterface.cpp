#include "serialportinterface.h"
#include "imu_trajectory_engine.h"
#include "qbytearray_datastream.h"
#include <QSerialPortInfo>
#include <QQuaternion>

SerialPortInterface::SerialPortInterface()
    : m_serialPort(new QSerialPort(this))
    , m_tcp(new QTcpSocket(this))
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
    qDebug() << "read buffer size" << m_serialPort->readBufferSize();
    connect(m_tcp, &QIODevice::readyRead, this, &SerialPortInterface::readPort);
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
        // if (!m_serialPort->isOpen()) {
        //     QList<QSerialPortInfo> port_list = QSerialPortInfo::availablePorts();
        //     for (QSerialPortInfo &serialPortInfo : port_list) {
        //         // qDebug() << "check port:" << serialPortInfo.portName();
        //         // qDebug() << "vendor identifier:" << serialPortInfo.vendorIdentifier();
        //         // qDebug() << "product identifier:" << serialPortInfo.productIdentifier();
        //         // if (serialPortInfo.vendorIdentifier() == 6790 &&
        //         //         serialPortInfo.productIdentifier() == 29987) // arduino uno
        //         if (serialPortInfo.vendorIdentifier() == 6790 &&
        //                 serialPortInfo.productIdentifier() == 29987) // LoRa Ebyte E22-400T22U
        //         {
        //             qDebug() << "set port...";
        //             m_serialPort->setPort(serialPortInfo);
        //             qDebug() << "success!";
        //             if (m_serialPort->open(QIODevice::ReadWrite)) {
        //                 qDebug() << "connected:" << m_serialPort->portName();
        //                 m_connectTimer->stop();
        //                 break;
        //             } else {
        //                 qDebug() << "connection error";
        //             }
        //         }
        //     }
        // }
        if (!m_tcp->isOpen()) {
            qDebug() << "try connect";
            m_tcp->connectToHost("192.168.1.202", 80);
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
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << stick_1_x << stick_1_y << stick_2_x << stick_2_y;
    sendPacket(MSG_ID_SET_MOTOR_SPEEDS, reinterpret_cast<uint8_t*>(data.data()), data.size());
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
    serialPort()->write(message);
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

void SerialPortInterface::setMotorSpeeds(uint16_t left_motor_pwm, uint16_t right_motor_pwm) {
    QByteArray data;
    QByteArrayWriter writer(data);
    writer << left_motor_pwm << right_motor_pwm;
    sendPacket(MSG_ID_SET_MOTOR_SPEEDS, reinterpret_cast<uint8_t*>(data.data()), data.size());
}

void SerialPortInterface::readPort() {
    // минимальный размер пакета: 4(header) + 1(crc) = 5

    // qDebug() << serialPort()->readAll();
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

    QByteArray data;
    uint8_t crc = header.function_code + header.size;
    if (header.size) {
        data = serialPort()->read(header.size);
        for (int i = 0; i < header.size; ++i) crc += data[i]; // подсчёт контрольной суммы
    }

    if (crc != getByte()) {
        qDebug() << "ошибка контрольной суммы";
        return;
    }

    // обработка данных
    switch(header.function_code) {
    case MSG_ID_ENC_CHANGED: {
        QByteArrayReader stream(data);
        float dl, dr;
        stream >> dl >> dr;

        static double x = 0, y = 0, f = 0;
        double b = 0.16;
        double d = (dr + dl) / 2.0;
        double df = (dr - dl) / b;
        double R = d / df;

        if (df != 0) {
            x = x + R * (sin(f + df) - sin(f));
            y = y - R * (cos(f + df) - cos(f));
            f = f + df;
        } else {
            x += d * cos(f);
        }

        // qDebug() << QString("x: %1, y: %2, f: %3").arg(QString::number(x, 'f', 4)).arg(QString::number(y, 'f', 4)).arg(QString::number(f * 180.0 / M_PI, 'f', 4));
        emit encPositionChanged(x, y, f);

        break;
    }
    case MSG_ID_IMU_CHANGED: {
        QByteArrayReader stream(data);
        float real, i, j, k, ax, ay, az;
        stream >> real >> i >> j >> k >> ax >> ay >> az;
        static TrajectoryEngine te;
        te.updateData(QQuaternion(real, i, j, k), QVector3D(ax, ay, az));
        qDebug() << te.position();
        emit imuPositionChanged(te.position().x(), te.position().y(), 0);
        break;
    }
    default:
        qDebug() << "неизвестный код функции:" << header.function_code;
        break;
    }

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
