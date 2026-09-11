#ifndef LIDARMAPVIEWER_H
#define LIDARMAPVIEWER_H

#include <QQuickPaintedItem>
#include <QWebSocket>

class LidarMapViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool trajectoryVisible READ trajectoryVisible WRITE setTrajectoryVisible NOTIFY trajectoryVisibleChanged)
    Q_PROPERTY(bool relative READ relative WRITE setRelative NOTIFY relativeChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    LidarMapViewer(QQuickItem *parent = nullptr);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; update(); }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }

    void setTrajectoryVisible(bool visible);
    inline bool trajectoryVisible() const { return m_trajectoryVisible; }
    Q_INVOKABLE void clearTrajectory();

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
        QList<uint8_t> data;
    };
    bool m_keepDevicePixelRatio;
    bool m_trajectoryVisible;
    bool m_relative;
    qreal m_zoom;
    // qreal m_yaw;
    QPoint m_currentCellPos;
    QVector3D m_currentPos;
    Map m_map;
    QList<QPointF> m_trajectory;
    QWebSocket *m_webSocket;
    void paint(QPainter *painter) override;

signals:
    void trajectoryVisibleChanged();
    void relativeChanged();
    void zoomChanged();
};
#endif // LIDARMAPVIEWER_H
