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
    , m_yaw(0)
    , m_map {
          .size_x = 0,
          .size_y = 0,
          .min_x = 0.0f,
          .min_y = 0.0f,
          .resolution = 0.0f,
          .data = {}
        }
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_targetPos(QPointF(NAN, NAN))
{
    setFillColor(QColor(90, 110, 100));
    // setFillColor(QColor(127, 127, 127));

    // setMipmap(true);
    // connect(window(), &QQuickWindow::visibilityChanged, this, &QQuickItem::update);

    // m_webSocket->open(QUrl("ws://192.168.0.0:8081/lidarMapViewer"));
    // m_webSocket->open(QUrl("ws://192.168.1.65:8081/lidarMapViewer"));
    // m_webSocket->open(QUrl("ws://192.168.31.21:8081/lidarMapViewer"));
    connect(m_webSocket, &QWebSocket::binaryMessageReceived, this, [this](const QByteArray &data) {
        QDataStream stream(data);
        stream >> m_map.size_x >> m_map.size_y >> m_map.min_x >> m_map.min_y >> m_map.resolution;
        // m_map.data = QList<uint8_t>(m_map.size_x * m_map.size_y);
        for (uint32_t i = 0; i < m_map.data.size(); ++i) {
            uint8_t cell;
            stream >> cell;
            m_map.data[i] = cell;
        }
        // qDebug() << m_map.min_x << m_map.min_y << m_map.resolution << m_map.size_x << m_map.size_y;
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
        m_yaw = yaw / M_PI * 180.0;
        m_currentPos = QPointF(x, y);
        m_trajectory << QPointF(x, y);
        update();
    });
    connect(interface, &WebSocketInterface::encPositionChanged, this, [this](float x, float y, float) {
        // qDebug() << QString("x: %1, y: %2, f: %3").arg(QString::number(x, 'f', 4)).arg(QString::number(y, 'f', 4)).arg(QString::number(yaw * 180.0 / M_PI, 'f', 4));
        m_encTrajectory << QPointF(x, y);
        update();
    });
    m_map.min_x = -4.28; m_map.min_y = -5.78; m_map.resolution = 0.05; m_map.size_x = 208; m_map.size_y = 254;
    m_map.data.resize(m_map.size_x * m_map.size_y);
    for (auto &cell : m_map.data) cell = 0;
    connect(interface, &WebSocketInterface::mapChanged, this,
            [this](uint32_t width, uint32_t height, float resolution,
            double ox, double oy, const QByteArray &cells_data) {
        // qDebug() << width << height << resolution << ox << oy;
        // qDebug() << cells_data;

        m_map.size_x = width;
        m_map.size_y = height;
        m_map.resolution = resolution;
        m_map.min_x = ox;
        m_map.min_y = oy;
        m_map.data = cells_data;
        // -4.28209 -5.78104 0.05 208 254
        // qDebug() << m_map.min_x << m_map.min_y << m_map.resolution << m_map.size_x << m_map.size_y;
        update();
    });
    connect(interface, &WebSocketInterface::waypointsUpdated, this, [this](const QList<QPointF> &waypoints) {
        m_waypoints = {m_currentPos};
        m_waypoints << waypoints;
    });
}

void LidarMapViewer::setTrajectoryVisible(bool visible) {
    if (m_trajectoryVisible == visible) return;
    m_trajectoryVisible = visible;
    emit trajectoryVisibleChanged();
    update();
}

void LidarMapViewer::clearTrajectory() {
    // if (m_trajectory.isEmpty()) return;
    m_trajectory.clear();
    m_encTrajectory.clear();
    m_targetPos = QPointF(NAN, NAN);
    m_waypoints.clear();
    emit trajectoryVisibleChanged();
    update();
}

void LidarMapViewer::setWaypointsMovement(bool state) {
    if (m_waypointsMovement == state) return;
    m_waypointsMovement = state;
    if (!m_waypointsMovement) m_waypoints.clear();
    emit waypointsMovementChanged();
}

void LidarMapViewer::clearWaypoints() {
    m_targetPos = QPointF(NAN, NAN);
    m_waypoints.clear();
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
    if (m_map.data.isEmpty()) return;

    if (m_keepDevicePixelRatio) {
        setTextureSize(QSize(width(), height()));
    } else {
        setTextureSize(QSize(width() * qApp->devicePixelRatio(), height() * qApp->devicePixelRatio()));
    }
    painter->translate(width(), 0.0);
    painter->scale(-1.0, 1.0);


    // size_x, size_y наоборот, так как карта повёрнута на 90 градусов
    const qreal map_ratio = qreal(m_map.size_y) / qreal(m_map.size_x);
    const qreal widget_ratio = width() / height();
    const qreal cell_size = m_zoom * (map_ratio > widget_ratio ? width() / qreal(m_map.size_y) : height() / qreal(m_map.size_x));

    const QPointF pos = QPointF(m_currentPos.x() - m_map.min_x, m_currentPos.y() - m_map.min_y) / m_map.resolution * cell_size;

    // painter->setRenderHint(QPainter::Antialiasing);
    painter->translate(QPointF(width(), height()) / 2.0);
    painter->rotate(-90.0);

    if (m_relative) {
        painter->rotate(-m_yaw);
        painter->translate(-pos);
    } else {
        painter->translate(QPointF(m_map.size_x, m_map.size_y) * -cell_size / 2.0);
    }
    m_painterTransform = painter->worldTransform().inverted();

    // painter->setBrush(Qt::blue);
    painter->setPen(Qt::NoPen);
    // painter->scale(-1.0, 1.0);

    // painter->drawPoints(m_scanPoints);
    for (uint32_t x = 0; x < m_map.size_x; ++x) {
        for (uint32_t y = 0; y < m_map.size_y; ++y) {
            // формула для доступа к ячейке: x + y * size_x
            const int8_t cell_data = uint8_t(m_map.data.at(x + y * m_map.size_x));
            QColor color;
            if (cell_data == -1) {
                color = QColor(90, 110, 100);
            } else {
                uint8_t gray = (1.f - float(cell_data) / 100.f) * 180.f;
                color = QColor(gray, gray, gray);
            }
            painter->setBrush(color);
            painter->drawRect(QRectF(x * cell_size, y * cell_size, cell_size * 1.001, cell_size * 1.001));

            // if (cell_data == 100) {
            //     const double mx = m_map.min_x + (x + 0.5) * m_map.resolution;
            //     const double my = m_map.min_y + (y + 0.5) * m_map.resolution;
            //     QPointF p = QPointF(mx - m_map.min_x, my - m_map.min_y) / m_map.resolution * cell_size;
            //     painter->setBrush(Qt::red);
            //     painter->drawEllipse(p, 3, 3);
            // }
        }
    }

    painter->setRenderHint(QPainter::Antialiasing);

    // отрисовка траектории
    if (m_trajectoryVisible) {
        painter->setPen(QPen(Qt::red, 2.0));
        for (qsizetype i = 1; i < m_encTrajectory.size(); ++i) {
            QPointF p1 = (m_encTrajectory.at(i-1) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            QPointF p2 = (m_encTrajectory.at( i ) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            painter->drawLine(p1, p2);
        }
        painter->setPen(QPen(Qt::blue, 2.0));
        for (qsizetype i = 1; i < m_trajectory.size(); ++i) {
            QPointF p1 = (m_trajectory.at(i-1) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            QPointF p2 = (m_trajectory.at( i ) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            painter->drawLine(p1, p2);
        }
        painter->setPen(QPen(Qt::green, 2.0));
        for (qsizetype i = 1; i < m_waypoints.size(); ++i) {
            QPointF p1 = (m_waypoints.at(i-1) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            QPointF p2 = (m_waypoints.at( i ) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size;
            painter->drawLine(p1, p2);
            // painter->drawEllipse((m_waypoints.at(i) - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size, 3, 3);
        }
    }
    painter->setPen(QPen(Qt::black, 0));
    painter->setBrush(Qt::green);
    if (!std::isnan(m_targetPos.x())) {
        painter->drawEllipse((m_targetPos - QPointF(m_map.min_x, m_map.min_y)) / m_map.resolution * cell_size, 3, 3);
    }
    painter->setBrush(Qt::red);
    painter->setPen(QPen(Qt::black, 1.2));
    painter->translate(pos);
    painter->rotate(m_yaw);

    QPolygonF arrow{QPointF(0.0, 0.0), QPointF(24.0, 10.0), QPointF(0.0, 20.0), QPointF(6.8, 10.0)};
    arrow.translate(-arrow.boundingRect().width() / 2.0, -arrow.boundingRect().height() / 2.0);
    painter->drawPolygon(arrow);
}

void LidarMapViewer::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        const qreal map_ratio = qreal(m_map.size_y) / qreal(m_map.size_x);
        const qreal widget_ratio = width() / height();
        const qreal cell_size = m_zoom * (map_ratio > widget_ratio ? width() / qreal(m_map.size_y) : height() / qreal(m_map.size_x));
        m_targetPos = m_painterTransform.map(event->position() * qApp->devicePixelRatio()) *
                m_map.resolution / cell_size + QPointF(m_map.min_x, m_map.min_y);
        auto interface = qobject_cast<WebSocketInterface*>(AppGlobal::instance()->interface());
        if (m_waypointsMovement) {
            // if (m_waypoints.empty()) m_waypoints << m_currentPos;
            // m_waypoints << m_targetPos;
            interface->addWaypoint(m_targetPos.x(), m_targetPos.y(), std::atan2(m_targetPos.y(), m_targetPos.x()));
        } else {
            interface->setWaypoint(m_targetPos.x(), m_targetPos.y(), std::atan2(m_targetPos.y(), m_targetPos.x()));
        }
        update();
    }
}
