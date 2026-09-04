import QtQuick as Q

Flickable {
    id: root
    required property Q.TextEdit textEdit
    clip: true
    contentWidth: Math.max(textEdit.implicitWidth, root.width)
    contentHeight: Math.max(textEdit.implicitHeight, root.height)
    boundsMovement: Flickable.StopAtBounds
    boundsBehavior: Flickable.DragAndOvershootBounds
    Q.Behavior on contentX { Q.NumberAnimation { duration: 100 } }
    Q.Behavior on contentY { Q.NumberAnimation { duration: 100 } }
    Q.Component.onCompleted: {
        textEdit.parent = contentItem
        textEdit.anchors.fill = contentItem
        textEdit.cursorPositionChanged.connect(scrollToCursor)
    }
    function scrollToCursor() {
        var speed = 500
        var padding = 4
        var viewRect = Qt.rect(root.contentX, root.contentY, root.width, root.height)

        if (viewRect.left + padding > textEdit.cursorRectangle.left) {
            root.contentX += -viewRect.left + textEdit.cursorRectangle.left - padding
        } else if (viewRect.right - padding < textEdit.cursorRectangle.right) {
            root.contentX += -viewRect.right + textEdit.cursorRectangle.right + padding
        }

        if (viewRect.top + padding > textEdit.cursorRectangle.top) {
            root.contentY += -viewRect.top + textEdit.cursorRectangle.top - padding
        } else if (viewRect.bottom - padding < textEdit.cursorRectangle.bottom) {
            root.contentY += -viewRect.bottom + textEdit.cursorRectangle.bottom + padding
        }
    }
    //onFlickEnded: textEdit.scrollToCursor()
}
