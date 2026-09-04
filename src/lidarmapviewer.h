#ifndef LIDARMAPVIEWER_H
#define LIDARMAPVIEWER_H

#include <QQuickPaintedItem>
#include <QWebSocket>

class LidarMapViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(bool trajectoryVisible READ trajectoryVisible WRITE setTrajectoryVisible NOTIFY trajectoryVisibleChanged)

public:
    LidarMapViewer(QQuickItem *parent = nullptr);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; update(); }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }
    void setTrajectoryVisible(bool visible);
    inline bool trajectoryVisible() const { return m_trajectoryVisible; }
    Q_INVOKABLE void clearTrajectory();;

private:
    bool m_keepDevicePixelRatio;
    bool m_trajectoryVisible;
    uint32_t m_sizeX;
    uint32_t m_sizeY;
    qreal m_yaw;
    QPoint m_currentCellPos;
    QList<uint8_t> m_cells;
    QList<QPoint> m_trajectory;
    QWebSocket *m_webSocket;
    void paint(QPainter *painter) override;

signals:
    void trajectoryVisibleChanged();
};
#endif // LIDARMAPVIEWER_H
