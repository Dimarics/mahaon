import QtQuick as Q

Flickable {
    id: root
    property alias font: textEdit.font
    property alias text: textEdit.text
    property alias textDocument: textEdit.textDocument
    property alias lineCount: textEdit.lineCount
    function insert(pos, text) { textEdit.insert() }
    property int currentLine: 1
    clip: true
    contentWidth: textEdit.width; contentHeight: textEdit.height
    Q.TextEdit {
        id: textEdit
        color: "black"
        selectionColor: Style.textSelectionColor
        font.pointSize: 14
        font.family: "consolas"
        padding: 10; leftPadding: 6
        //Q.Keys.onTabPressed: insert(cursorPosition, "    ")
        Q.Keys.forwardTo: root
        width: Math.max(implicitWidth, root.width); height: Math.max(implicitHeight, root.height)
        onCursorPositionChanged: {
            var line = 1, pos = 0;
            while ((pos = text.indexOf('\n', pos) + 1) > 0 && pos <= cursorPosition) {
                line++;
            }
            root.currentLine = line;
            scrollToCursor()
        }
        function scrollToCursor() {
            var speed = 500
            var padding = 4
            var viewRect = Qt.rect(root.contentX, root.contentY, root.width, root.height)

            if (viewRect.left + padding > cursorRectangle.left) {
                root.contentX += -viewRect.left + cursorRectangle.left - padding
            } else if (viewRect.right - padding < cursorRectangle.right) {
                root.contentX += -viewRect.right + cursorRectangle.right + padding
            }

            if (viewRect.top + padding > cursorRectangle.top) {
                root.contentY += -viewRect.top + cursorRectangle.top - padding
            } else if (viewRect.bottom - padding < cursorRectangle.bottom) {
                root.contentY += -viewRect.bottom + cursorRectangle.bottom + padding
            }
        }
    }
    Q.Behavior on contentX { Q.NumberAnimation { duration: 100 } }
    Q.Behavior on contentY { Q.NumberAnimation { duration: 100 } }
    //onFlickEnded: textEdit.scrollToCursor()
}
