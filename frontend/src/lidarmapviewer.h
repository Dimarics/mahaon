#ifndef LIDARMAPVIEWER_H
#define LIDARMAPVIEWER_H

#include <QQuickPaintedItem>
#include <QWebSocket>

class LidarMapViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool scanVisible READ scanVisible WRITE setScanVisible NOTIFY scanVisibleChanged)
    Q_PROPERTY(bool trajectoryVisible READ trajectoryVisible WRITE setTrajectoryVisible NOTIFY trajectoryVisibleChanged)
    Q_PROPERTY(bool waypointsMovement READ waypointsMovement WRITE setWaypointsMovement NOTIFY waypointsMovementChanged)
    Q_PROPERTY(bool relative READ relative WRITE setRelative NOTIFY relativeChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    LidarMapViewer(QQuickItem *parent = nullptr);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; update(); }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }

    void setScanVisible(bool visible);
    inline bool scanVisible() const { return m_scanVisible; }

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
    struct OccupancyGridMap {
        uint32_t width;
        uint32_t height;
        float origin_x;
        float origin_y;
        float resolution;
        qreal cell_pixel_size;
        QByteArray data;
        inline QPointF metersToPixels(const QPointF meters) const {
            return (meters - QPointF(origin_x, origin_y)) / resolution * cell_pixel_size;
        }
        inline QPointF pixelsToMeters(const QPointF pixels) const {
            return pixels * resolution / cell_pixel_size + QPointF(origin_x, origin_y);
        }
    };
    bool m_keepDevicePixelRatio;
    bool m_scanVisible;
    bool m_trajectoryVisible;
    bool m_waypointsMovement;
    bool m_relative;
    qreal m_zoom;
    qreal m_yaw;
    QPointF m_currentPos;
    OccupancyGridMap m_map;
    QList<QPointF> m_trajectory;
    QList<QPointF> m_encTrajectory;
    QList<QPointF> m_waypoints;
    QList<QPointF> m_scan;
    QWebSocket *m_webSocket;

    QPointF m_targetPos;
    QTransform m_painterTransform;

    void paint(QPainter *painter) override;
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void updateMapCellSize();

signals:
    void scanVisibleChanged();
    void trajectoryVisibleChanged();
    void waypointsMovementChanged();
    void relativeChanged();
    void zoomChanged();
};
#endif // LIDARMAPVIEWER_H
