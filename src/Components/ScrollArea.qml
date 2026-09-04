import QtQuick as Q
//import QtQuick.Controls.Basic as C
//import Hobots

Q.Item {
    id: root
    enum WheelDir { Vertical, Horizontal }
    default property alias content: contentItem.data
    property int wheelDir: ScrollArea.Vertical
    property real contentX: 0
    property real contentY: 0
    property alias contentWidth: contentItem.width
    property alias contentHeight: contentItem.height
    property Q.color sliderColor: "gray"
    property ScrollBar verticalScrollBar: verticalScrollBar
    property ScrollBar horizontalScrollBar: horizontalScrollBar
    //clip: true
    /*MouseHandler {
        property bool pressed: false
        property point mousePress
        width: root.width; height: root.height
        onPressed: (point, button) => {
            if (button === Qt.LeftButton) {
                pressed = true
                mousePress.x = point.x - contentX
                mousePress.y = point.y - contentY
            }
        }
        onPositionChanged: point => {
            if (pressed) {
                contentX = point.x - mousePress.x; contentY = point.y - mousePress.y
            }
        }
        onReleased: pressed = false
    }*/
    onWidthChanged: {
        if (contentX > contentWidth - root.width) contentX = contentWidth - root.width
        else if (contentX < 0) contentX = 0
    }
    onHeightChanged: {
        if (contentY > contentHeight - root.height) contentY = contentHeight - root.height
        else if (contentY < 0) contentY = 0
    }
    onContentXChanged: {
        if (mouseArea.pressed) return
        horizontalScrollBar.position = contentX / (contentWidth - width) * (1 - horizontalScrollBar.size)
    }
    onContentYChanged: {
        if (mouseArea.pressed) return
        verticalScrollBar.position = contentY / (contentHeight - height) * (1 - verticalScrollBar.size)
    }
    Q.MouseArea {
        id: mouseArea
        property point mousePress
        width: root.width; height: root.height
        acceptedButtons: Qt.LeftButton
        onPressed: {
            mousePress.x = mouseX + root.contentX
            mousePress.y = mouseY + root.contentY
        }
        onPositionChanged: {
            if (pressed) {
                var newX = mousePress.x - mouseX
                var newY = mousePress.y - mouseY
                if (newX > root.contentWidth - root.width) newX = contentWidth - root.width
                else if (newX < 0) newX = 0
                if (newY > root.contentHeight - root.height) newY = contentHeight - root.height
                else if (newY < 0) newY = 0
                root.contentX = newX; root.contentY = newY
                //
                horizontalScrollBar.position = root.contentX / (root.contentWidth - width) * (1 - horizontalScrollBar.size)
                verticalScrollBar.position = root.contentY / (root.contentHeight - height) * (1 - verticalScrollBar.size)
            }
        }
        onWheel: wheel => {
                     var delta = wheel.angleDelta.y / 120 * 20
                     if (root.wheelDir === ScrollArea.Vertical) {
                         root.contentY -= delta
                     } else {
                         root.contentX -= delta
                     }
                 }
    }
    Q.Item {
        id: contentItem
        x: -root.contentX; y: -root.contentY
        width: root.width; height: root.height
        /*Q.Rectangle {
            z: 1
            anchors.fill: parent
            color: "green"
        }
        Q.Rectangle {
            z: 1
            x: 400; y: 20
            width: 200; height: 100
            color: "red"
        }*/
    }
    ScrollBar {
        id: verticalScrollBar
        color: root.sliderColor

        x: root.width - width
        height: root.height
        //position: 0

        orientation: Qt.Vertical
        bottomPadding: horizontalScrollBar.visible ? horizontalScrollBar.height : 2
        size: root.height / root.contentHeight
        onPositionChanged: {
            if (!pressed) return
            root.contentY = (root.contentHeight - root.height) * position / (1 - size)
        }
    }
    ScrollBar {
        id: horizontalScrollBar
        color: root.sliderColor

        y: root.height - height
        width: root.width
        //position: 0

        orientation: Qt.Horizontal
        rightPadding: verticalScrollBar.visible ? verticalScrollBar.width : 2
        size: root.width / root.contentWidth
        onPositionChanged: {
            if (!pressed) return
            root.contentX = (root.contentWidth - root.width) * position / (1 - size)
        }
    }
    Q.Component.onCompleted: {
        horizontalScrollBar.position = contentX / (contentWidth - width) * (1 - horizontalScrollBar.size)
        verticalScrollBar.position = contentY / (contentHeight - height) * (1 - verticalScrollBar.size)
    }
}
