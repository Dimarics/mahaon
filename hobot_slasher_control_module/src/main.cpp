// #include "BNO085.h"
// #include "sh2.h"
// #include "lidarviewer.h"
#include <QCoreApplication>
#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QElapsedTimer>
#include "controller.h"
// #include "rosscansubscriber.h"
// #include <GPS_ATGM336.h>
// #include <TinyGPS++.h>
// #include <QQuaternion>

// float normalizeAngle(float yaw) {
//     if (yaw >= 360) {
//         return yaw - 360;
//     } else if (yaw < 0) {
//         return 360 + yaw;
//     }
//     return yaw;
// }

// void quaternionToEuler(sh2_RotationVectorWAcc_t *rv) {
//     float yaw, pitch, roll;
//     QQuaternion quaternion(rv->real, rv->i, rv->j, rv->k);
//     quaternion.getEulerAngles(&roll, &pitch, &yaw);

//     qDebug() << "===================";
//     qDebug() << "roll:" << roll;
//     qDebug() << "pitch:" << pitch;
//     qDebug() << "yaw:" << yaw;
// }

int main(int argc, char *argv[])
{
    // rclcpp::init(argc, argv);                    // 1. Инициализация ROS2
    // rclcpp::spin(std::make_shared<LidarSubscriber>()); // 2. Блокирующий цикл обработки
    // rclcpp::shutdown();

    QCoreApplication app(argc, argv);
    // ======================= Mapper ============================
    Controller controller;

    // ======================= Lidar =============================
    // Lidar lidar;
    // LidarViewer lidarViewer;
    // lidarViewer.showMaximized();
    // lidarViewer.show();

    // QObject::connect(&lidar, &Lidar::scanChanged, &app, [](std::shared_ptr<LaserScan> scan) {
    //     qDebug() << QString("Получено сканирование[%1]: %2 точек, частота [%3] Гц")
    //                 .arg(scan->stamp).arg((unsigned int)scan->points.size()).arg(1.0 / scan->config.scan_time);
    //     // QList<QPointF> points;
    //     // for (LaserPoint &scanPoint : scan->points) {
    //     //     qreal angle = scanPoint.angle - M_PI_2;
    //     //     points << scanPoint.range * QPointF(cos(angle), sin(angle)) * 100.f;
    //     // }
    //     // lidarViewer.setPoints(points);
    // });

    // ======================== GPS ==============================

    // TinyGPSPlus gps;
    // for (const QSerialPortInfo &serialPortInfo : QSerialPortInfo::availablePorts()) {
    //     qDebug() << serialPortInfo.portName();
    // }
    // QSerialPort serialPort("COM3");
    // qDebug() << "connected:" << serialPort.open(QIODevice::ReadWrite);
    // GPS_ATGM336 gps_settings(&serialPort);
    // gps_settings.system(GPS_GP, GPS_GL);
    // gps_settings.composition(NMEA_GGA, NMEA_RMC);
    // gps_settings.model(GPS_PORTABLE);
    // gps_settings.updaterate(5);
    // QObject::connect(&serialPort, &QIODevice::readyRead, &serialPort, [&gps, &serialPort] {
    //     char c;
    //     while (serialPort.getChar(&c)) {
    //         // std::cout << c;
    //         if (gps.encode(c)) {
    //             // Данные обновлены, можно их использовать.
    //             qDebug() << "sattelites:" << gps.satellites.value();
    //             if (gps.location.isValid()) {
    //                 double latitude = gps.location.lat();
    //                 double longitude = gps.location.lng();
    //                 qDebug() << "latitude:" << latitude << "longitude:" << longitude;
    //             }
    //         }
    //     }
    // });

    return app.exec();

    // ===================== BNO085 =======================

    // Adafruit_BNO08x bno085;
    // TwoWire wire;
    // // wire.begin();
    // //qDebug() << "connection:" << wire.isConnected(0x4B);
    // if (bno085.begin_I2C(&wire, 0x4B)) {
    //     qDebug() << "BNO08x connected";
    //     sh2_SensorValue_t sensorValue;
    //     bno085.enableReport(SH2_ARVR_STABILIZED_RV, 5000);

    //     while (true) {
    //         if (bno085.getSensorEvent(&sensorValue)) {
    //             switch (sensorValue.sensorId) {
    //             case SH2_ARVR_STABILIZED_RV:
    //                 quaternionToEuler(&sensorValue.un.arvrStabilizedRV);
    //                 break;
    //             case SH2_GYROSCOPE_CALIBRATED:
    //                 // sensorValue.un.gyroscope.z;
    //                 break;
    //             }
    //         }
    //     }
    // }
}
