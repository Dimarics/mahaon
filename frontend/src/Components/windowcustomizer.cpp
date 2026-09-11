#include "windowcustomizer.h"

#ifdef Q_OS_WINDOWS
static const int margin = 9;
typedef BOOL (WINAPI *PFN_ISWINDOWARRANGED)(HWND);
static PFN_ISWINDOWARRANGED isWindowArranged = NULL;
WNDPROC WindowCustomizer::WndProc;

LRESULT CALLBACK WindowCustomizer::CustomizerWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_EXITSIZEMOVE:
        if (isWindowArranged && isWindowArranged(hWnd)) {
            RECT rc;
            GetWindowRect(hWnd, &rc);
            int x = rc.left - margin;
            int y = rc.top - margin;
            int width = rc.right - rc.left + margin * 2;
            int height = rc.bottom - rc.top + margin * 2;
            MoveWindow(hWnd, x, y, width, height, true);
        }
        break;
    case WM_NCCALCSIZE:
        if (wParam) {
            NCCALCSIZE_PARAMS* pParams = (NCCALCSIZE_PARAMS*)lParam;
            pParams->rgrc[0].top += margin;
            pParams->rgrc[0].bottom -= margin;
            pParams->rgrc[0].left += margin;
            pParams->rgrc[0].right -= margin;
        }
        return 0;
    case WM_NCACTIVATE:
        return wParam ? 0 : 1;
    case WM_NCPAINT: {
        // Получаем контекст устройства для неклиентской области
        HDC hdc = GetWindowDC(hWnd);

        // Получаем размеры неклиентской области
        RECT rc;
        GetWindowRect(hWnd, &rc);
        OffsetRect(&rc, -rc.left, -rc.top);

        // Рисуем кастомный заголовок
        //HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
        //FillRect(hdc, &rc, hBrush);
        //DeleteObject(hBrush);
        QImage shadow(rc.right, rc.bottom, QImage::Format_ARGB32);
        int clWidth = rc.right - margin * 2;
        int clHeight = rc.bottom - margin * 2;
        shadow.fill(Qt::transparent);

        QLinearGradient leftShadow(0, 0, 1, 0);
        leftShadow.setCoordinateMode(QGradient::ObjectMode);
        leftShadow.setColorAt(0, Qt::transparent);
        leftShadow.setColorAt(0.5, QColor(0, 0, 0, 25));
        leftShadow.setColorAt(1, QColor(0, 0, 0, 70));

        QRadialGradient leftTopShadow(1, 1, 1);
        leftTopShadow.setCoordinateMode(QGradient::ObjectMode);
        leftTopShadow.setColorAt(0, QColor(0, 0, 0, 60));
        leftTopShadow.setColorAt(0.5, QColor(0, 0, 0, 20));
        leftTopShadow.setColorAt(1, Qt::transparent);

        QLinearGradient topShadow(0, 0, 0, 1);
        topShadow.setCoordinateMode(QGradient::ObjectMode);
        topShadow.setColorAt(0, Qt::transparent);
        topShadow.setColorAt(0.5, QColor(0, 0, 0, 25));
        topShadow.setColorAt(1, QColor(0, 0, 0, 70));

        QRadialGradient rightTopShadow(0, 1, 1);
        rightTopShadow.setCoordinateMode(QGradient::ObjectMode);
        rightTopShadow.setColorAt(0, QColor(0, 0, 0, 60));
        rightTopShadow.setColorAt(0.5, QColor(0, 0, 0, 20));
        rightTopShadow.setColorAt(1, Qt::transparent);

        QLinearGradient rightShadow(0, 0, 1, 0);
        rightShadow.setCoordinateMode(QGradient::ObjectMode);
        rightShadow.setColorAt(0, QColor(0, 0, 0, 70));
        rightShadow.setColorAt(0.5, QColor(0, 0, 0, 25));
        rightShadow.setColorAt(1, Qt::transparent);

        QRadialGradient rightBottomShadow(0, 0, 1);
        rightBottomShadow.setCoordinateMode(QGradient::ObjectMode);
        rightBottomShadow.setColorAt(0, QColor(0, 0, 0, 60));
        rightBottomShadow.setColorAt(0.5, QColor(0, 0, 0, 20));
        rightBottomShadow.setColorAt(1, Qt::transparent);

        QLinearGradient bottomShadow(0, 0, 0, 1);
        bottomShadow.setCoordinateMode(QGradient::ObjectMode);
        bottomShadow.setColorAt(0, QColor(0, 0, 0, 70));
        bottomShadow.setColorAt(0.5, QColor(0, 0, 0, 25));
        bottomShadow.setColorAt(1, Qt::transparent);

        QRadialGradient leftBottomShadow(1, 0, 1);
        leftBottomShadow.setCoordinateMode(QGradient::ObjectMode);
        leftBottomShadow.setColorAt(0, QColor(0, 0, 0, 60));
        leftBottomShadow.setColorAt(0.5, QColor(0, 0, 0, 20));
        leftBottomShadow.setColorAt(1, Qt::transparent);

        QPainter painter(&shadow);
        painter.fillRect(0, margin, margin, clHeight, leftShadow);
        painter.fillRect(0, 0, margin, margin, leftTopShadow);
        painter.fillRect(margin, 0, clWidth, margin, topShadow);
        painter.fillRect(rc.right - margin, 0, margin, margin, rightTopShadow);
        painter.fillRect(rc.right - margin, margin, margin, clHeight, rightShadow);
        painter.fillRect(rc.right - margin, rc.bottom - margin, margin, margin, rightBottomShadow);
        painter.fillRect(margin, rc.bottom - margin, clWidth, margin, bottomShadow);
        painter.fillRect(0, rc.bottom - margin, margin, margin, leftBottomShadow);
        painter.end();

        BITMAPINFO bmi;
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = rc.right;
        bmi.bmiHeader.biHeight = -rc.bottom;  // Отрицательная высота для top-down изображения
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;     // 32 бита на пиксель (RGBA)
        bmi.bmiHeader.biCompression = BI_RGB;

        void* pixelData;
        HBITMAP hBitmap = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pixelData, NULL, 0);
        for (int x = 0; x < shadow.width(); ++x) {
            for (int y = 0; y < shadow.height(); ++y) {
                QRgb color = shadow.pixel(x, y);
                float a = qAlpha(color);
                float r = qRed(color) * a / 255.f;
                float g = qGreen(color) * a / 255.f;
                float b = qBlue(color) * a / 255.f;
                //unsigned long color = qRgba(255.f * alpha / 255.f, 0, 0, alpha);
                ((COLORREF*)pixelData)[y * rc.right + x] = qRgba(r, g, b, a);
            }
        }
        HDC hdcMem = CreateCompatibleDC(hdc);
        SelectObject(hdcMem, hBitmap);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, hdcMem, 0, 0, SRCCOPY);
        DeleteObject(hBitmap);
        DeleteObject(hdcMem);

        // Освобождаем контекст устройства
        ReleaseDC(hWnd, hdc);
        return 0;
    }
    default:
        break;
    }
    return WndProc(hWnd, msg, wParam, lParam);
    //return DefWindowProc(hWnd, msg, wParam, lParam);
}

/*LRESULT CALLBACK WindowCustomizer::CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    qDebug() << nCode;
    switch (nCode) {
    case WM_NCCALCSIZE:
    {
        // Корректируем клиентскую область
        if (wParam)
        {
            NCCALCSIZE_PARAMS* pParams = (NCCALCSIZE_PARAMS*)lParam;
            pParams->rgrc[0].top += margin;
            pParams->rgrc[0].bottom -= margin;
            pParams->rgrc[0].left += margin;
            pParams->rgrc[0].right -= margin;
            return 0;
        }
        break;
    }
    default:
        break;
    }
    return 0;
}*/
#endif

WindowCustomizer::WindowCustomizer(QObject *parent) : QObject(parent), m_window(nullptr) {
#ifdef Q_OS_WINDOWS
    if (!isWindowArranged) {
        HMODULE hUser32 = GetModuleHandle(L"user32.dll");
        if (hUser32) isWindowArranged = (PFN_ISWINDOWARRANGED)GetProcAddress(hUser32, "IsWindowArranged");
    }
#endif
}

void WindowCustomizer::setWindow(QWindow *window)
{
#ifdef Q_OS_WINDOWS
    if (m_window == window) return;
    m_window = window;
    if (window) {
        HWND hWnd = (HWND)window->winId();
        //DWM
        //DWMNCRENDERINGPOLICY policy = DWMNCRP_DISABLED;
        //DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
        /*MARGINS margins;
            margins.cyTopHeight = margin;
            margins.cyBottomHeight = margin;
            margins.cxLeftWidth = margin;
            margins.cxRightWidth = margin;
            DwmExtendFrameIntoClientArea(hWnd, &margins);*/
        //
        WndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)CustomizerWndProc);
        //RedrawWindow(hWnd, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        //MoveWindow(hWnd, window->x(), window->y(), window->width(), window->height(), true);
        //SetWindowPos(hWnd, NULL, window->x(), window->y(), window->width(), window->height(),);
    }
#elif defined(Q_OS_LINUX)
#elif defined(Q_OS_ANDROID)
#elif defined(Q_OS_MACOS)
#endif
    emit windowChanged();
}
