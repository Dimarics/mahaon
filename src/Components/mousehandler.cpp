#include "mousehandler.h"
#include "qquickwindow.h"

MouseHandler::MouseHandler(QQuickItem *parent) : QQuickItem(parent) {
    setAcceptedMouseButtons(Qt::LeftButton);
    //setAcceptHoverEvents(true);
    //qApp->installEventFilter(this);
    connect(this, &QQuickItem::windowChanged, this, [this](QQuickWindow *window) {
        if (window) window->installEventFilter(this);
    });
}

MouseHandler::~MouseHandler() {
    if (window()) window()->removeEventFilter(this);
}

bool MouseHandler::eventFilter(QObject *target, QEvent *event)
{
    //if (target != window()) return QQuickItem::eventFilter(target, event);
    QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
    switch (event->type()) {
    case QEvent::MouseButtonPress:
        if (acceptedMouseButtons() & mouseEvent->buttons())
            emit pressed(mapFromGlobal(mouseEvent->globalPosition()), mouseEvent->button());
        break;
    case QEvent::MouseMove:
        if (acceptHoverEvents() || acceptedMouseButtons() & mouseEvent->buttons())
            emit positionChanged(mapFromGlobal(mouseEvent->globalPosition()), mouseEvent->button());
        break;
    case QEvent::MouseButtonRelease:
        if (acceptedMouseButtons() & mouseEvent->buttons())
            emit released(mapFromGlobal(mouseEvent->globalPosition()), mouseEvent->button());
        break;
    /*case QEvent::TouchBegin: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->pointCount()) {
            emit pressed(mapFromGlobal(touchEvent->point(0).globalPosition()), Qt::NoButton);
        }
    }
        break;
    case QEvent::TouchEnd: {
        QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);
        if (touchEvent->pointCount()) {
            emit released(mapFromGlobal(touchEvent->point(0).globalPosition()), Qt::NoButton);
        }
    }
        break;*/
    default:
        break;
    }
    return QQuickItem::eventFilter(target, event);
}
