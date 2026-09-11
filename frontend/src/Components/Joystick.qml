import QtQuick as Q
import QtQuick.Shapes

Q.Rectangle {
    id: root
    implicitWidth: 120; implicitHeight: 120
    property real fromX: 0; property real toX: 1
    property real fromY: 0; property real toY: 1
    property real startX: 0.5; property real startY: 0.5
    readonly property real valueX: stick.x / width * (toX - fromX) + fromX
    readonly property real valueY: -(stick.y / height * (toY - fromY) - toY)
    property alias active: drag.active
    // width / (toX - fromX) * (valueX - fromX) = x
    // valueY = -(stick.y / height * (toY - fromY) - toY + fromY)
    signal positionChanged
    color: Style.joystickBkgColor
    border.color: Style.joystickBorderColor
    border.width: 1
    // Q.MouseArea {
    //     id: mouseArea
    //     anchors.fill: parent
    //     onPositionChanged: function(event)  {
    //         if (tap.active) {
    //             var new_x = event.x - tap.point.pressPosition.x
    //             var new_y = event.y - tap.point.pressPosition.y
    //             if (new_x < 0) {
    //                 new_x = 0
    //             } else if (new_x > root.width) {
    //                 new_x = root.width
    //             }
    //             if (new_y < 0) {
    //                 new_y = 0
    //             } else if (new_y > root.height) {
    //                 new_y = root.height
    //             }
    //             stick.x = new_x
    //             stick.y = new_y
    //             root.positionChanged()
    //         }
    //     }
    //     onReleased: {
    //         stick.x = root.width / (root.toX - root.fromX) * (root.startX - root.fromX)
    //         stick.y = root.height / (root.toY - root.fromY) * (root.toY - root.startY)
    //         root.positionChanged()
    //     }
    // }
    Shape {
        id: stick
        containsMode: Shape.FillContains
        preferredRendererType: Shape.CurveRenderer
        property real radius: 20
        x: root.width / (root.toX - root.fromX) * (root.startX - root.fromX)
        y: root.height / (root.toY - root.fromY) * (root.toY - root.startY)
        ShapePath {
            strokeWidth: 2
            strokeColor: Style.joystickBorderColor
            fillColor: drag.active ? Style.joystickBkgPressedColor : Style.joystickBkgHoverColor
            //startX: stick.radius; startY: stick.radius
            Q.PathAngleArc {
                centerX: 0; centerY: 0
                radiusX: stick.radius; radiusY: stick.radius
                startAngle: 0; sweepAngle: 360
            }
        }
        // Q.PointHandler {
        //     id: tap
        //     // onActiveChanged: mouseArea.forceActiveFocus()
        // }
        Q.DragHandler {
            id: drag
            target: stick
            dragThreshold: 0
            xAxis.minimum: 0
            xAxis.maximum: root.width
            yAxis.minimum: 0
            yAxis.maximum: root.height
            onActiveChanged: {
                if (!active) {
                    stick.x = root.width / (root.toX - root.fromX) * (root.startX - root.fromX)
                    stick.y = root.height / (root.toY - root.fromY) * (root.toY - root.startY)
                    root.positionChanged()
                }
            }
            onTranslationChanged: root.positionChanged()
        }
    }
}
