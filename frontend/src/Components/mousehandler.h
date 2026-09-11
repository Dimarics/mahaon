#ifndef MOUSEHANDLER_H
#define MOUSEHANDLER_H

#include <QQuickItem>
#include <QCursor>

class MouseHandler : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(Qt::CursorShape cursorShape READ cursorShape WRITE setCursorShape NOTIFY cursorShapeChanged)
public:
    inline void setCursorShape(Qt::CursorShape shape) { setCursor(QCursor(shape)); emit cursorShapeChanged(); }
    inline Qt::CursorShape cursorShape() const { return cursor().shape(); }
    MouseHandler(QQuickItem *parent = nullptr);
    ~MouseHandler();

protected:
    bool eventFilter(QObject *target, QEvent *event) override;

signals:
    void pressed(const QPointF &pos, Qt::MouseButton button);
    void positionChanged(const QPointF &pos, Qt::MouseButton button);
    void released(const QPointF &pos, Qt::MouseButton button);
    void cursorShapeChanged();
};

#endif // MOUSEHANDLER_H
