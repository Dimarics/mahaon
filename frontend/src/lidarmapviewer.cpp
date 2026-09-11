#include "lidarmapviewer.h"
#include "appglobal.h"
#include "websocketinterface.h"
#include <QPainter>
#include <QTimer>
#include <QQuickWindow>

LidarMapViewer::LidarMapViewer(QQuickItem *parent)
    : QQuickPaintedItem(parent)
    , m_keepDevicePixelRatio(true)
    , m_trajectoryVisible(false)
    , m_relative(false)
    , m_zoom(1.0)
    , m_map {
          .size_x = 0,
          .size_y = 0,
          .min_x = 0.0f,
          .min_y = 0.0f,
          .resolution = 0.0f,
          .data = {}
          }
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    setFillColor(QColor(127, 127, 127));
    // setMipmap(true);
    connect(window(), &QQuickWindow::visibilityChanged, this, &QQuickItem::update);

    m_webSocket->open(QUrl("ws://192.168.0.0:8081/lidarMapViewer"));
    // m_webSocket->open(QUrl("ws://192.168.1.65:8081/lidarMapViewer"));
    // m_webSocket->open(QUrl("ws://192.168.31.21:8081/lidarMapViewer"));
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &data) {
        QDataStream stream(data);
        stream >> m_map.size_x >> m_map.size_y >> m_map.min_x >> m_map.min_y >> m_map.resolution;
        m_map.data = QList<uint8_t>(m_map.size_x * m_map.size_y);
        for (uint32_t i = 0; i < m_map.data.size(); ++i) {
            uint8_t cell;
            stream >> cell;
            m_map.data[i] = cell;
        }
        // qDebug() << m_map.min_x << m_map.min_y << m_map.resolution;
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

    //-------------------------------------------------------
    auto interface = qobject_cast<WebSocketInterface*>(AppGlobal::instance()->interface());
    // connect(interface, &WebSocketInterface::mapCellPositionChanged, this, [this](const QPoint cellPos) {
    //     m_currentCellPos = cellPos;
    //     m_trajectory << cellPos;
    //     update();
    // });
    connect(interface, &WebSocketInterface::localPositionChanged, this, [this](float x, float y, float yaw) {
        // qDebug() << yaw / M_PI * 180.0;
        // m_yaw = yaw / M_PI * 180.0;
        m_currentPos = QVector3D(x, y, yaw / M_PI * 180.0);
        m_trajectory << QPointF(x, y);
        update();
    });
}

void LidarMapViewer::setTrajectoryVisible(bool visible) {
    if (m_trajectoryVisible == visible) return;
    m_trajectoryVisible = visible;
    emit trajectoryVisibleChanged();
    update();
}

void LidarMapViewer::clearTrajectory() {
    if (m_trajectory.isEmpty()) return;
    m_trajectory.clear();
    emit trajectoryVisibleChanged();
    update();
}

void LidarMapViewer::setRelative(bool relative) {
    if (m_relative == relative) return;
    m_relative = relative;
    emit relativeChanged();
    update();
}

void LidarMapViewer::setZoom(qreal zoom) {
    if (m_zoom == zoom) return;
    m_zoom = zoom;
    emit zoomChanged();
    update();
}

void LidarMapViewer::paint(QPainter *painter) {
    if (m_map.data.empty()) return;

    if (m_keepDevicePixelRatio) {
        setTextureSize(QSize(width(), height()));
    } else {
        setTextureSize(QSize(width() * qApp->devicePixelRatio(), height() * qApp->devicePixelRatio()));
    }
    // painter->translate(width(), 0.0);
    // painter->scale(-1.0, 1.0);

    const qreal k_cells = qreal(m_map.size_x) / qreal(m_map.size_y);
    const qreal k_widget = width() / height();
    const qreal cell_size = m_zoom * (k_cells > k_widget ? width() / qreal(m_map.size_x) : height() / qreal(m_map.size_y));
    const QPointF pos = QPointF(m_currentPos.x() - m_map.min_x, m_currentPos.y() - m_map.min_y) / m_map.resolution * cell_size;

    // painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(QPointF(width(), height()) / 2.0);
    painter->rotate(-90.0);
    if (m_relative) {
        painter->rotate(-m_currentPos.z());
        painter->translate(-pos);
    } else {
        painter->translate(QPointF(m_map.size_x, m_map.size_y) * -cell_size / 2.0);
    }
    // painter->setBrush(Qt::blue);
    painter->setPen(Qt::NoPen);
    // painter->scale(-1.0, 1.0);

    // painter->drawPoints(m_scanPoints);
    for (uint32_t x = 0; x < m_map.size_x; ++x) {
        for (uint32_t y = 0; y < m_map.size_y; ++y) {
            // формула для доступа к ячейке: x + y * m_sizeX
            const uint8_t gray = m_map.data.at(x + y * m_map.size_x);
            painter->setBrush(QColor(gray, gray, gray));
            painter->drawRect(QRectF(x * cell_size, y * cell_size, cell_size * 1.001, cell_size * 1.001));
        }
    }

    painter->setRenderHint(QPainter::Antialiasing);

    // отрисовка траектории
    if (m_trajectoryVisible) {
        painter->setPen(QPen(Qt::blue, 2.0));
        for (qsizetype i = 1; i < m_trajectory.size(); ++i) {
            // QPointF p1(cell_size * (m_trajectory.at(i-1).x() + 0.5), cell_size * (m_trajectory.at(i-1).y() + 0.5));
            // QPointF p2(cell_size * (m_trajectory.at( i ).x() + 0.5), cell_size * (m_trajectory.at( i ).y() + 0.5));
            // painter->drawLine(p1, p2);
            QPointF p1 = (m_trajectory.at(i-1) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            QPointF p2 = (m_trajectory.at( i ) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            painter->drawLine(p1, p2);
        }
    }
    painter->setBrush(Qt::red);
    painter->setPen(QPen(Qt::black, 1.2));
    if (m_relative) {
        painter->translate(pos);
        painter->rotate(m_currentPos.z());
    } else {
        painter->translate(pos);
        painter->rotate(m_currentPos.z());
    }
    QPolygonF arrow{QPointF(0.0, 0.0), QPointF(24.0, 10.0), QPointF(0.0, 20.0), QPointF(6.8, 10.0)};
    arrow.translate(-arrow.boundingRect().width() / 2.0, -arrow.boundingRect().height() / 2.0);
    painter->drawPolygon(arrow);
}
