import QtQuick as Q

Q.Flickable {
    id: root
    property Q.color sliderColor: "white"
    clip: true
    height: 90; width: parent.width
    flickableDirection: Flickable.AutoFlickIfNeeded
    ScrollBar.horizontal: ScrollBar {
        color: sliderColor
        rightPadding: root.ScrollBar.vertical.visible ? root.ScrollBar.vertical.width : 2
    }
    ScrollBar.vertical: ScrollBar {
        color: sliderColor
        bottomPadding: root.ScrollBar.horizontal.visible ? root.ScrollBar.horizontal.height : 2
    }
    /*Q.PointHandler {
        onActiveChanged: if (active) parent.focus = true
    }*/
}
