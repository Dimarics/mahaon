#ifndef LIDARVIEWER_H
#define LIDARVIEWER_H

#include <QQuickPaintedItem>
#include <QWebSocket>

class LidarViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

public:
    LidarViewer(QQuickItem *parent = nullptr);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }
    void setZoom(qreal zoom);
    inline qreal zoom() const { return m_zoom; }

private:
    bool m_keepDevicePixelRatio;
    qreal m_zoom;
    QList<QPointF> m_scanPoints;
    QWebSocket *m_webSocket;

    void paint(QPainter *painter) override;
    void mousePressEvent(QMouseEvent *event) override;

signals:
    void zoomChanged();
};

#endif // LIDARVIEWER_H
