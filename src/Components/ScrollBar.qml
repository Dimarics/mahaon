import QtQuick
import QtQuick.Templates as T

T.ScrollBar {
    id: scrollBar
    property alias color: slider.color
    policy: T.ScrollBar.AsNeeded

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    minimumSize: orientation === Qt.Horizontal ? height / width : width / height

    padding: 2
    rightPadding: orientation == Qt.Vertical ? 4 : 0
    bottomPadding: orientation == Qt.Horizontal ? 4 : 0
    visible: {
        switch(policy) {
        case T.ScrollBar.AlwaysOff:
            return false
        case T.ScrollBar.AlwaysOn:
            return true
        case T.ScrollBar.AsNeeded:
            return size < 1.0
        }
        return false
    }
    contentItem: Rectangle {
        id: slider
        implicitWidth: Style.scrollBarWidth; implicitHeight: Style.scrollBarWidth
        radius: Style.scrollBarCornerRadius
        color: Style.scrollBarColor
        opacity: scrollBar.pressed ? 0.45 : scrollBar.hovered ? 0.3 : 0.2
        Behavior on opacity { OpacityAnimator { duration: 300 } }
    }
}
