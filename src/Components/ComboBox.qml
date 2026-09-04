import QtQuick as Q
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ComboBox {
    id: root
    //enabled: delegateModel && delegateModel.count > 1
    font.pointSize: Style.textSize
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    //implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
    //                         implicitContentHeight + topPadding + bottomPadding,
    //                         implicitIndicatorHeight + topPadding + bottomPadding)
    implicitHeight: 30

    leftPadding: 12; rightPadding: 12; topPadding: 4; bottomPadding: 4
    background: Q.Rectangle {
        radius: height / 2
        color: Style.comboBoxBkgColor
        border.width: 1
        border.color: pressed ? Style.comboBoxBorderPressedColor : hovered ? Style.comboBoxBorderHoverColor : Style.comboBoxBorderColor
        Q.Behavior on border.color { Q.ColorAnimation { duration: 100 } }
    }
    contentItem: Text {
        rightPadding: root.indicator.width + root.spacing
        text: root.displayText
        font: root.font
        color: pressed ? Style.comboBoxTextPressedColor : hovered ? Style.comboBoxTextHoverColor : Style.comboBoxTextColor
    }
    indicator: ColorImage  {
        x: root.width - width - 10
        y: root.topPadding + (root.availableHeight - height) / 2 + 2
        width: 13; height: 8
        color: "#90A6C0"
        //color: "white"
        //color: hovered ? "white" : "#90A6C0"
        //defaultColor: "#c8c8c8"
        source: "qrc:/images/down-arrow.svg"
        sourceSize: Qt.size(width, height)
        //source: "qrc:/qt-project.org/imports/QtQuick/Controls/Universal/images/downarrow.png"
        //source: "qrc:/qt-project.org/imports/QtQuick/Controls/Fusion/images/arrow.png"
        //source: "qrc:/qt-project.org/imports/QtQuick/Controls/Material/images/drop-indicator.png"
    }
    delegate: T.ItemDelegate {
        id: delegate
        required property var model
        required property int index
        width: Q.ListView.view.width; height: root.height
        leftPadding: root.leftPadding
        contentItem: Text {
            text: delegate.model[root.textRole]
            color: Style.comboBoxTextColor
            font: root.font
        }
        background: Q.Rectangle {
            id: highlight
            states: Q.State {
                when: highlightedIndex === index
                Q.PropertyChanges {
                    target: highlight
                    color: Style.listViewBkgHighlightColor
                    //border.color: Style.brightWidgetBorderColor
                }
            }
            color: "transparent"
            border.color: "transparent"
        }
    }
    popup: T.Popup {
        leftPadding: 1
        rightPadding: 1
        topPadding: 15
        bottomPadding: 15
        padding: 0
        width: root.width
        //implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
        //                        contentWidth + leftPadding + rightPadding)
        implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                                 contentHeight + topPadding + bottomPadding)
        contentItem: Q.ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.delegateModel
            currentIndex: root.highlightedIndex
            //populate: Q.Transition { Q.NumberAnimation { properties: "x,y"; duration: 100 } }
        }
        background: Q.Rectangle {
            color: root.background.color
            border.color: root.background.border.color
            radius: 15
        }
        onVisibleChanged: if (visible && (!root.delegateModel || !root.delegateModel.count)) visible = false
    }
}
