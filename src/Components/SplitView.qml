import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.Basic as B

T.SplitView {
    id: root
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    handle: Rectangle {
        implicitWidth: root.orientation === Qt.Horizontal ? 2 : root.width
        implicitHeight: root.orientation === Qt.Horizontal ? root.height : 2
        //color: "transparent"
        color: Style.backgroundColor_1
        //color: T.SplitHandle.pressed ? Style.darkBorderColor
        //    : (T.SplitHandle.hovered ? Style.brightBorderColor : Style.brightBorderColor)
    }
}
