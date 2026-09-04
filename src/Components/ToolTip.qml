import QtQuick as Q
import QtQuick.Controls.Basic as T

T.ToolTip {
    id: root
    delay: 1000
    timeout: 5000
    leftPadding: 6; rightPadding: 6; topPadding: 4; bottomPadding: 4
    background: Q.Rectangle {
        color: Style.toolTipBkgColor
        border.color: Style.toolTipBorderColor
        radius: Style.toolTipCornerRadius
    }
    contentItem: Text {
        id: label
        text: root.text
        font.pointSize: 10
    }
}
