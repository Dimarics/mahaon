#include "lidarviewer.h"
#include "appglobal.h"
#include "websocketinterface.h"
#include <QPainter>
#include <QTimer>
#include <QQuickWindow>

LidarViewer::LidarViewer(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_keepDevicePixelRatio(true)
    , m_zoom(1.0)
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    // setMipmap(true);
    setFillColor(QColor(0, 0, 0));
    connect(window(), &QQuickWindow::visibilityChanged, this, &QQuickItem::update);

    m_webSocket->open(QUrl("ws://192.168.0.0:8081/lidarViewer"));
    // m_webSocket->open(QUrl("ws://192.168.1.65:8081/lidarViewer"));
    // m_webSocket->open(QUrl("ws://192.168.31.21:8081/lidarViewer"));
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &data) {
        QDataStream stream(data);
        QList<QPointF> scan_points;
        stream >> scan_points;
        // qDebug() << scan_points.size();
        if (scan_points.isEmpty()) return;
        m_scanPoints = scan_points;
        update();
    });
    QTimer *connectTimer = new QTimer(this);
    connectTimer->setInterval(1000);
    connect(connectTimer, &QTimer::timeout, this, [this] {
        if (m_webSocket->state() == QAbstractSocket::UnconnectedState) {
            m_webSocket->open(m_webSocket->requestUrl());
        }
    });
    connectTimer->start();
}

void LidarViewer::setZoom(qreal zoom) {
    if (m_zoom == zoom) return;
    m_zoom = zoom;
    emit zoomChanged();
    update();
}

void LidarViewer::paint(QPainter *painter)
{
    if (m_keepDevicePixelRatio) {
        setTextureSize(QSize(width(), height()));
    } else {
        setTextureSize(QSize(width() * qApp->devicePixelRatio(), height() * qApp->devicePixelRatio()));
    }

    painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(width() / 2.0, height() / 2.0);

    const qreal k = qreal(width() > height() ? height() : width()) / 8.4 / 2.0 * m_zoom;

    // разметка
    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(QColor(127, 127, 127), 0));
    // окружности
    painter->drawEllipse(QPointF(0.0, 0.0), 2.0 * k, 2.0 * k);
    painter->drawEllipse(QPointF(0.0, 0.0), 4.0 * k, 4.0 * k);
    painter->drawEllipse(QPointF(0.0, 0.0), 6.0 * k, 6.0 * k);
    painter->drawEllipse(QPointF(0.0, 0.0), 8.0 * k, 8.0 * k);
    // линии
    for (uint8_t i = 0; i < 6; ++i) {
        const qreal angle = i * M_PI / 6.0;
        const qreal x = -8.0 * k * cos(angle);
        const qreal y =  8.0 * k * sin(angle);
        painter->drawLine(QLineF(-x, -y, x, y));
    }

    //
    painter->rotate(-90);
    painter->setBrush(Qt::green);
    painter->setPen(Qt::NoPen);
    // painter->drawPoints(m_scanPoints);

    for (QPointF &scan_point : m_scanPoints) {
        painter->drawEllipse(scan_point * k, 1.6, 1.6);
    }

    painter->setBrush(Qt::red);
    painter->setPen(QPen(Qt::white, 1.2));
    // painter->drawEllipse(QPointF(0, 0), 3.6, 3.6);
    // width: 24; height: 28.8
    QPolygonF arrow{QPointF(0.0, 0.0), QPointF(24.0, 10.0), QPointF(0.0, 20.0), QPointF(6.8, 10.0)};
    arrow.translate(-arrow.boundingRect().width() / 2.0, -arrow.boundingRect().height() / 2.0);
    painter->drawPolygon(arrow);
}

void LidarViewer::mousePressEvent(QMouseEvent *event) {
    const qreal k = qreal(width() > height() ? height() : width()) / 8.4 / 2.0 * m_zoom;
    const qreal x = -(event->pos().y() - height() / 2.0) / k;
    const qreal y = -(event->pos().x() - width() / 2.0) / k;
    // qDebug() << x << y << std::atan2(y, x) / M_PI * 180.0;
    auto interface = qobject_cast<WebSocketInterface*>(AppGlobal::instance()->interface());
    interface->setWaypoint(x, y, std::atan2(y, x), true);
}
