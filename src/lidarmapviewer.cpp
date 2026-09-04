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
    , m_sizeX(0)
    , m_sizeY(0)
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
        stream >> m_sizeX >> m_sizeY;
        m_cells = QList<uint8_t>(m_sizeX * m_sizeY);
        for (uint32_t i = 0; i < m_cells.size(); ++i) {
            uint8_t cell;
            stream >> cell;
            m_cells[i] = cell;
        }
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
    connect(interface, &WebSocketInterface::mapCellPositionChanged, this, [this](const QPoint cellPos) {
        m_currentCellPos = cellPos;
        m_trajectory << cellPos;
        update();
    });
    connect(interface, &WebSocketInterface::localPositionChanged, this, [this](float, float, float yaw) {
        // qDebug() << yaw / M_PI * 180.0;
        m_yaw = yaw / M_PI * 180.0;
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

void LidarMapViewer::paint(QPainter *painter)
{
    if (!m_sizeX || !m_sizeY) return;

    if (m_keepDevicePixelRatio) {
        setTextureSize(QSize(width(), height()));
    } else {
        setTextureSize(QSize(width() * qApp->devicePixelRatio(), height() * qApp->devicePixelRatio()));
    }

    const qreal k_cells = qreal(m_sizeX) / qreal(m_sizeY);
    const qreal k_widget = width() / height();
    const qreal cell_size = k_cells > k_widget ? width() / qreal(m_sizeX) : height() / qreal(m_sizeY);

    // painter->setRenderHint(QPainter::Antialiasing);
    painter->translate((width() - cell_size * m_sizeX) / 2.0, (height() - cell_size * m_sizeY) / 2.0);
    // painter->setBrush(Qt::blue);
    painter->setPen(Qt::NoPen);

    // painter->drawPoints(m_scanPoints);
    for (uint32_t x = 0; x < m_sizeX; ++x) {
        for (uint32_t y = 0; y < m_sizeY; ++y) {
            // формула для доступа к ячейке: x + y * m_sizeX
            const uint8_t gray = m_cells.at(x + y * m_sizeX);
            painter->setBrush(QColor(gray, gray, gray));
            painter->drawRect(QRectF(x * cell_size, y * cell_size, cell_size * 1.001, cell_size * 1.001));
        }
    }

    painter->setRenderHint(QPainter::Antialiasing);

    // отрисовка траектории
    if (m_trajectoryVisible) {
        painter->setPen(QPen(Qt::blue, 2.0));
        for (qsizetype i = 1; i < m_trajectory.size(); ++i) {
            QPointF p1(cell_size * (m_trajectory.at(i-1).x() + 0.5), cell_size * (m_trajectory.at(i-1).y() + 0.5));
            QPointF p2(cell_size * (m_trajectory.at( i ).x() + 0.5), cell_size * (m_trajectory.at( i ).y() + 0.5));
            painter->drawLine(p1, p2);
        }
    }

    // painter->setPen(Qt::NoPen);
    // painter->setBrush(Qt::red);
    // painter->drawEllipse(QPointF(m_currentCellPos) * cell_size + QPointF(cell_size, cell_size) / 2.0, 3.6, 3.6);
    painter->setBrush(Qt::red);
    painter->setPen(QPen(Qt::black, 1.2));
    painter->translate(cell_size * (m_currentCellPos.x() + 0.5), cell_size * (m_currentCellPos.y() + 0.5));
    painter->rotate(m_yaw);
    QPolygonF arrow{QPointF(0.0, 0.0), QPointF(24.0, 10.0), QPointF(0.0, 20.0), QPointF(6.8, 10.0)};
    arrow.translate(-arrow.boundingRect().width() / 2.0, -arrow.boundingRect().height() / 2.0);
    painter->drawPolygon(arrow);
}
