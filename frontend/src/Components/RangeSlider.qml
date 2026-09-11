import QtQuick
import QtQuick.Controls.Basic as T

T.RangeSlider {
    id: control
    background: Rectangle {
        x: control.leftPadding + (control.horizontal ? 0 : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : 0)
        implicitWidth: control.horizontal ? 200 : 6
        implicitHeight: control.horizontal ? 6 : 200
        width: control.horizontal ? control.availableWidth : implicitWidth
        height: control.horizontal ? implicitHeight : control.availableHeight
        radius: 3
        color: Style.sliderVoidColor
        border.color: Style.sliderBorderColor
        Rectangle {
            x: control.horizontal ? control.first.visualPosition * control.width : 0
            y: control.horizontal ? 0 : control.first.visualPosition * control.height
            width: control.horizontal ? (control.second.visualPosition - control.first.visualPosition) * control.width : 6
            height: control.horizontal ? 6 : (control.second.visualPosition - control.first.visualPosition) * control.height
            radius: 3
            color: Style.sliderFillColor
            border.color: Style.sliderBorderColor
        }
    }
    first.handle: Rectangle {
        x: control.leftPadding + (control.horizontal ? control.first.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.first.visualPosition * (control.availableHeight - height))
        implicitWidth: 15; implicitHeight: 15
        radius: 4
        color: Style.sliderHandleColor
        //border.width: control.visualFocus ? 2 : 1
        border.color: Style.sliderHandleBorderColor
    }
    second.handle: Rectangle {
        x: control.leftPadding + (control.horizontal ? control.second.visualPosition * (control.availableWidth - width) : (control.availableWidth - width) / 2)
        y: control.topPadding + (control.horizontal ? (control.availableHeight - height) / 2 : control.second.visualPosition * (control.availableHeight - height))
        implicitWidth: 15; implicitHeight: 15
        radius: 4
        color: Style.sliderHandleColor
        border.color: Style.sliderHandleBorderColor
    }
}
