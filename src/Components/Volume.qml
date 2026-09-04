import QtQuick

Item {
    id: root
    property alias min: numberField.min
    property alias max: numberField.max
    property alias value: numberField.value
    property alias decimals: numberField.decimals
    property real spacing: 6
    signal valueModified

    implicitHeight: Math.max(numberField.implicitHeight, slider.implicitHeight)
    implicitWidth: numberField.implicitWidth + spacing + slider.implicitWidth
    Component.onCompleted: slider.value = value

    NumberField {
        id: numberField
        min: 0; max: 1; value: 0
        anchors.verticalCenter: root.verticalCenter
        onValueModified: {
            slider.value = numberField.value
            root.valueModified()
        }
    }
    Slider {
        id: slider
        x: numberField.width + root.spacing
        width: root.width - x
        anchors.verticalCenter: root.verticalCenter
        padding: 0
        from: root.min; to: root.max
        onMoved: {
            value = value.toFixed(root.decimals)
            numberField.value = slider.value
        }
        onPressedChanged: {
            if (!pressed) root.valueModified()
        }
    }
}
