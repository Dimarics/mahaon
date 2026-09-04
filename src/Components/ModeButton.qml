import QtQuick

Button {
    id: root
    borderWidth: 3
    width: 200; height: 300
    scale: pressed ? 0.97 : 1
    contentItem: Item {
        Image {
            width: icon.width; height: icon.height
            source: icon.source
            sourceSize: Qt.size(width, height)
            fillMode: Image.PreserveAspectFit
            mipmap: true
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            anchors { centerIn: parent; verticalCenterOffset: -20 }
        }
        Text {
            text: root.text
            color: root.pressed ? Style.buttonTextPressedColor : root.hovered ? Style.buttonTextHoverColor : Style.buttonTextColor
            font.pointSize: Style.textSize
            //font.weight: Font.DemiBold
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
        }
    }
}
