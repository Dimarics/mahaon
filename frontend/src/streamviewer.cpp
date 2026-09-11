#include "streamviewer.h"
#include <QTimer>
#include <QTcpSocket>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/opencv.hpp>

#ifdef __EMSCRIPTEN__
#include "appbackend.h"
#endif

StreamViewer::StreamViewer(QQuickItem *parent) : ImageViewer(parent),
    m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    //m_tcpSocket(new QTcpSocket(this))
{
    setKeepDevicePixelRatio(false);

#ifdef __EMSCRIPTEN__
    QMetaObject::invokeMethod(this, [this] {
        m_webSocket->open(QUrl(QString("ws://%1:%2/followStream").arg(AppBackend::instance()->webServerAddress()).arg(8081)));
    }, Qt::QueuedConnection);
#else
    // m_webSocket->open(QUrl("ws://192.168.1.65:8081/followStream"));
    m_webSocket->open(QUrl("ws://192.168.31.21:8081/followStream"));
#endif
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &frame) {
        setImageFromData(frame);
        //m_webSocket->sendBinaryMessage("OK");
        //cv::Mat mat = cv::imdecode(std::vector<uchar>(frame.begin(), frame.end()), cv::IMREAD_COLOR);
        //cv::imshow("a", mat);
    });
    QTimer *connectTimer = new QTimer(this);
    connectTimer->setInterval(1000);
    connect(connectTimer, &QTimer::timeout, this, [this] {
        if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
            m_webSocket->open(m_webSocket->requestUrl());
        }
    });
    connectTimer->start();

/*
    m_tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
#ifdef __EMSCRIPTEN__
    QMetaObject::invokeMethod(this, [this] {
        qDebug() << AppBackend::instance()->webServerAddress();
        m_tcpSocket->connectToHost(AppBackend::instance()->webServerAddress(), 8082);
    }, Qt::QueuedConnection);
#else
    m_tcpSocket->connectToHost("localhost", 8082);
#endif
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, [this] {
        //qDebug() << m_tcpSocket->readAll();
        static qsizetype size = 0;
        if (!size && m_tcpSocket->bytesAvailable() >= 4) {
            uint8_t data[4] { 0, 0, 0, 0 };
            m_tcpSocket->read(reinterpret_cast<char*>(data), 4);
            size = uint32_t(data[0]) << 24 | uint32_t(data[1]) << 16 | uint16_t(data[2]) << 8 | uint8_t(data[3]);
            //qDebug() << size;
        }
        if (m_tcpSocket->bytesAvailable() >= size) {
            static QElapsedTimer tm;
            // qDebug() << tm.elapsed() << size;
            tm.restart();
            QByteArray frame = m_tcpSocket->read(size);
            setImageFromData(frame);
            // m_tcpSocket->write("OK");
            // m_tcpSocket->flush();
            size = 0;
            // cv::Mat mat = cv::imdecode(std::vector<uchar>(frame.begin(), frame.end()), cv::IMREAD_COLOR);
            // cv::imshow("a", mat);
        }
    });
    connect(m_tcpSocket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError error) {
        qDebug() << error;
        QTimer::singleShot(1000, this, [this] {
            if (m_tcpSocket->state() == QAbstractSocket::UnconnectedState) {
                m_tcpSocket->connectToHost("localhost", 8082);
            }
        });
    });
    */
}
