import QtQuick
import QtQuick.Controls.Basic as T
import QtQuick.Controls.impl
import QtQuick.Layouts

T.CheckBox {
    id: root
    hoverEnabled: Qt.platform.os !== "android"
    display: T.AbstractButton.TextUnderIcon
    spacing: 8
    icon.width: 40; icon.height: 40
    icon.color: pressed ? Style.radioButtonTextPressedColor : hovered ?
                          Style.radioButtonTextHoverColor : Style.radioButtonTextColor
    font.pointSize: Style.textSize
    /*
    background: Rectangle {
        id: backgroundRect
        //radius: Style.buttonCornerRadius
        border.width: 0
        Behavior on color { ColorAnimation { duration: 200 } }
        Behavior on border.color { ColorAnimation { duration: 100 } }
    }
    */
    indicator: Rectangle {
        implicitWidth: 16
        implicitHeight: 16

        x: root.text ? (root.mirrored ? root.width - width - root.rightPadding : root.leftPadding) : root.leftPadding + (root.availableWidth - width) / 2
        y: root.topPadding + (root.availableHeight - height) / 2

        radius: width / 2
        color: Style.radioButtonBkgColor
        border.width: 1
        border.color: pressed ? Style.radioButtonBorderPressedColor : hovered ?
                                    Style.radioButtonBorderHoverColor : Style.radioButtonBorderColor
        Rectangle {
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            width: 8
            height: 8
            radius: width / 2
            color: "white"//root.palette.text
            visible: root.checked
        }
    }
    contentItem: CheckLabel {
        leftPadding: root.indicator && !root.mirrored ? root.indicator.width + root.spacing : 0
        rightPadding: root.indicator && root.mirrored ? root.indicator.width + root.spacing : 0
        text: root.text
        font: root.font
        color: root.icon.color
    }
    //containmentMask: RoundedRectMask { radius: borderRadius; width: root.width; height: root.height }
    Behavior on icon.color { ColorAnimation { duration: 100 } }
}
