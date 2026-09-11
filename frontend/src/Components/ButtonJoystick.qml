import QtQuick as Q
import QtQuick.Templates as T
import QtQuick.Shapes

Q.Item {
    id: root
    width: 120; height: 120
    property alias leftText: leftText.text
    property alias leftStatePressed: left.pressed
    property alias rightText: rightText.text
    property alias rightStatePressed: right.pressed
    property alias topText: topText.text
    property alias topStatePressed: top.pressed
    property alias bottomText: bottomText.text
    property alias bottomStatePressed: bottom.pressed
    signal leftPressed; signal leftReleased
    signal rightPressed; signal rightReleased
    signal topPressed; signal topReleased
    signal bottomPressed; signal bottomReleased
    component JoystickButton: T.Button {
        id: button
        property real startAngle
        containmentMask: shape
        hoverEnabled: Qt.platform.os !== "android"
        background: Shape {
            id: shape
            containsMode: Shape.FillContains
            preferredRendererType: Shape.CurveRenderer
            width: root.width; height: root.height
            ShapePath {
                strokeWidth: Style.joystickBorderWidth
                //strokeColor: button.pressed ? Style.buttonPressedBorderColor :
                //                       button.hovered ? Style.buttonHoveredBorderColor : Style.buttonBorderColor
                strokeColor: Style.joystickBorderColor
                //fillColor: button.pressed ? Style.buttonPressedBackgroundColor :
                //                     button.hovered ? Style.buttonHoveredBackgroundColor : Style.buttonBackgroundColor
                fillColor: button.pressed ? Style.joystickBkgPressedColor : button.hovered ? Style.joystickBkgHoverColor : Style.joystickBkgColor
                startX: root.width / 2; startY: root.height / 2
                Q.PathAngleArc {
                    centerX: root.width / 2; centerY: root.height / 2
                    radiusX: root.width / 2; radiusY: root.height / 2
                    startAngle: button.startAngle; sweepAngle: 90
                    moveToStart: false
                }
                Q.PathLine { x: root.width / 2; y: root.height / 2 }
                Q.Behavior on fillColor { Q.ColorAnimation { duration: 100 } }
            }
        }
    }
    Q.FontLoader {
        id: joystickFont
        source: "qrc:/fonts/NotoSans-Regular.ttf"
    }
    component JoystickText: Text {
        scale: parent.pressed ? 0.9 : 1
        font.weight: Q.Font.DemiBold
        font.pointSize: 11
        font.family: joystickFont.font.family
        color: parent.pressed ? Style.joystickTextPressedColor : parent.hovered ? Style.joystickTextHoverColor : Style.joystickTextColor
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
    JoystickButton {
        id: left
        startAngle: 135
        onPressed: leftPressed(); onReleased: leftReleased()
        JoystickText {
            id: leftText
            x: (root.width / 4 - implicitWidth) / 2
            height: root.height
        }
    }
    JoystickButton {
        id: right
        startAngle: 315
        onPressed: rightPressed(); onReleased: rightReleased()
        JoystickText {
            id: rightText
            x: root.width / 8 * 7 - implicitWidth / 2
            height: root.height
            horizontalAlignment: Text.AlignRight
        }
    }
    JoystickButton {
        id: top
        startAngle: 225
        onPressed: topPressed(); onReleased: topReleased()
        JoystickText {
            id: topText
            y: (root.height / 4 - implicitHeight) / 2
            width: root.width
        }
    }
    JoystickButton {
        id: bottom
        startAngle: 45
        onPressed: bottomPressed(); onReleased: bottomReleased()
        JoystickText {
            id: bottomText
            y: root.height / 8 * 7 - implicitHeight / 2
            width: root.width
        }
    }
    Shape {
        containsMode: Shape.FillContains
        preferredRendererType: Shape.CurveRenderer
        anchors.centerIn: parent
        ShapePath {
            strokeWidth: Style.joystickBorderWidth
            //strokeColor: button.pressed ? Style.buttonPressedBorderColor :
            //                       button.hovered ? Style.buttonHoveredBorderColor : Style.buttonBorderColor
            strokeColor: Style.joystickBorderColor
            fillColor: Style.joystickBkgColor
            startX: root.width / 2; startY: root.height / 2
            Q.PathAngleArc {
                centerX: root.width / 4; centerY: root.height / 4
                radiusX: root.width / 4; radiusY: root.height / 4
                startAngle: 0; sweepAngle: 360
            }
        }
    }
}
