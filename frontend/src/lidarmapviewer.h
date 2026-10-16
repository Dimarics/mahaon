#ifndef LIDARMAPVIEWER_H
#define LIDARMAPVIEWER_H

#include <QQuickPaintedItem>
#include <QWebSocket>

class LidarMapViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool trajectoryVisible READ trajectoryVisible WRITE setTrajectoryVisible NOTIFY trajectoryVisibleChanged)
    Q_PROPERTY(bool waypointsMovement READ waypointsMovement WRITE setWaypointsMovement NOTIFY waypointsMovementChanged)
    Q_PROPERTY(bool relative READ relative WRITE setRelative NOTIFY relativeChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    LidarMapViewer(QQuickItem *parent = nullptr);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; update(); }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }

    void setTrajectoryVisible(bool visible);
    inline bool trajectoryVisible() const { return m_trajectoryVisible; }
    Q_INVOKABLE void clearTrajectory();
    void setWaypointsMovement(bool state);
    inline bool waypointsMovement() const { return m_waypointsMovement; }
    Q_INVOKABLE void clearWaypoints();

    inline bool relative() const { return m_relative; }
    void setRelative(bool relative);

    inline qreal zoom() const { return m_zoom; }
    void setZoom(qreal zoom);

private:
    struct Map {
        uint32_t size_x;
        uint32_t size_y;
        float min_x;
        float min_y;
        float resolution;
        QByteArray data;
    };
    bool m_keepDevicePixelRatio;
    bool m_trajectoryVisible;
    bool m_waypointsMovement;
    bool m_relative;
    qreal m_zoom;
    qreal m_yaw;
    QPointF m_currentPos;
    Map m_map;
    QList<QPointF> m_trajectory;
    QList<QPointF> m_encTrajectory;
    QList<QPointF> m_waypoints;
    QWebSocket *m_webSocket;

    QPointF m_targetPos;
    QTransform m_painterTransform;

    void paint(QPainter *painter) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void trajectoryVisibleChanged();
    void waypointsMovementChanged();
    void relativeChanged();
    void zoomChanged();
};
#endif // LIDARMAPVIEWER_H
