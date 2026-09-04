import QtQuick

Item {
    id: root
    property real min: 0
    property real max: 100
    property real value: 0
    property real decimals: 2
    property real spacing: 6
    property alias firstValue: field_first.value
    property alias secondValue: field_second.value
    signal firstModified
    signal secondModified

    implicitHeight: Math.max(field_first.implicitHeight, slider.implicitHeight)
    implicitWidth: field_first.implicitWidth + slider.implicitWidth + field_second.implicitWidth + spacing * 2
    //Component.onCompleted: slider.value = value

    NumberField {
        id: field_first
        min: root.min
        max: root.max
        value: root.value
        decimals: root.decimals
        anchors.verticalCenter: root.verticalCenter
        onValueModified: {
            slider.first.value = field_first.value
            root.firstModified()
        }
    }
    RangeSlider {
        id: slider
        x: field_first.width + root.spacing
        width: root.width - x
        anchors.verticalCenter: root.verticalCenter
        padding: 0
        from: root.min; to: root.max
        first.value: root.firstValue
        second.value: root.secondValue
        first.onMoved: {
            first.value = first.value.toFixed(root.decimals)
            field_first.value = slider.first.value
        }
        first.onPressedChanged: {
            if (!first.pressed) root.firstModified()
        }
        second.onMoved: {
            second.value = second.value.toFixed(root.decimals)
            field_second.value = slider.second.value
        }
        second.onPressedChanged: {
            if (!second.pressed) root.secondModified()
        }
    }
    NumberField {
        id: field_second
        x: field_first.width + slider.width + root.spacing * 2
        min: root.min
        max: root.max
        value: root.value
        decimals: root.decimals
        anchors.verticalCenter: root.verticalCenter
        onValueModified: {
            slider.second.value = field_second.value
            root.secondModified()
        }
    }
}
