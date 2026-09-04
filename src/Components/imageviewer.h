#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QQuickPaintedItem>
#include <QImage>

class ImageViewer : public QQuickPaintedItem
{
    Q_OBJECT
    QML_ELEMENT
public:
    ImageViewer(QQuickItem *parent = nullptr);
    void setImageFromData(const QByteArray &data);
    inline void setKeepDevicePixelRatio(bool value) { m_keepDevicePixelRatio = value; }
    inline bool keepDevicePixelRatio() const { return m_keepDevicePixelRatio; }

private:
    QImage m_image;
    bool m_keepDevicePixelRatio;
    void paint(QPainter *painter) override;
};

#endif // IMAGEVIEWER_H
