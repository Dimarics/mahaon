#include "imageviewer.h"
#include <QPainter>

ImageViewer::ImageViewer(QQuickItem *parent) : QQuickPaintedItem(parent), m_keepDevicePixelRatio(true)
{
    //setFillColor(Qt::blue);
    setMipmap(true);
}

void ImageViewer::setImageFromData(const QByteArray &data)
{
    QImage image;
    image.loadFromData(data);
    if (!image.isNull()) m_image = image;
    update();
}

void ImageViewer::paint(QPainter *painter)
{
    if (m_image.isNull()) return;
    if (m_keepDevicePixelRatio) {
        setTextureSize(QSize(width(), height()));
    } else {
        setTextureSize(QSize(width() * qApp->devicePixelRatio(), height() * qApp->devicePixelRatio()));
    }

    // QImage drawImage(m_image.scaled(width() * k, height() * k, Qt::KeepAspectRatio));
    // int x = (width() - drawImage.width()) / 2.f;
    // int y = (height() - drawImage.height()) / 2.f;
    // painter->scale(1.0 / k, 1.0 / k);
    // painter->drawImage(x, y, drawImage);

    float i_ratio = float(m_image.width()) / float(m_image.height());
    float w_ratio = float(width()) / float(height());
    int x, y;
    float scale;
    if (i_ratio > w_ratio) {
        scale = float(width()) / float(m_image.width());
    } else {
        scale = float(height()) / float(m_image.height());
    }
    x = (width() / scale - m_image.width()) / 2.f;
    y = (height() / scale - m_image.height()) / 2.f;
    painter->scale(scale, scale);
    painter->drawImage(x, y, m_image);

    //painter->drawImage(0, 0, m_image);
}
