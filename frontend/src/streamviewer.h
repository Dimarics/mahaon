#ifndef STREAMVIEWER_H
#define STREAMVIEWER_H

#include "imageviewer.h"
#include <QWebSocket>

class StreamViewer : public ImageViewer
{
    Q_OBJECT
    QML_ELEMENT
public:
    StreamViewer(QQuickItem *parent = nullptr);

private:
    QWebSocket *m_webSocket;
    //QTcpSocket *m_tcpSocket;
};

#endif // STREAMVIEWER_H
