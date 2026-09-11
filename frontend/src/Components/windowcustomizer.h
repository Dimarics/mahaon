#ifndef WINDOWCUSTOMIZER_H
#define WINDOWCUSTOMIZER_H

#include <qqml.h>
#include <QPainter>
#include <QObject>
#include <QWindow>

#ifdef Q_OS_WINDOWS
#include <windows.h>
#include <dwmapi.h>
#endif

//extern Q_DECL_IMPORT void qt_blurImage(QPainter *painter, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

class WindowCustomizer : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QWindow* window READ window WRITE setWindow NOTIFY windowChanged)

public:
    WindowCustomizer(QObject *parent = nullptr);
    void setWindow(QWindow *window);
    inline QWindow *window() const { return m_window; };

private:
#ifdef Q_OS_WINDOWS
    static WNDPROC WndProc;
    static LRESULT CALLBACK CustomizerWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    //static LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam);
#endif
    QWindow* m_window;

signals:
    void windowChanged();
};

#endif // WINDOWCUSTOMIZER_H
