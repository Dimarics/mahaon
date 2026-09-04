import QtQuick as Q
import QtQuick.Controls.Basic as T
import QtQuick.Layouts

Q.Rectangle {
    id: root
    property int currentIndex: 0
    property list<string> model
    property alias tabWidth: highlight.implicitWidth
    width: layout.implicitWidth; height: layout.implicitHeight
    radius: height / 2
    color: Style.comboBoxBkgColor
    clip: true
    border.color: "transparent"
    Q.Component {
        id: button
        T.Button {
            id: control
            //checkable: true
            //autoExclusive: true
            background: null
            Layout.preferredWidth: highlight.width
            contentItem: Text {
                horizontalAlignment: Text.AlignHCenter
                text: control.text
            }
            onClicked: {
                for (let i = 0; i < layout.children.length; ++i)
                    if (layout.children[i] === this)
                        root.currentIndex = i;
            }
        }
    }
    Q.Rectangle {
        id: highlight
        x: layout.children[root.currentIndex] ? layout.children[root.currentIndex].x : 0
        implicitWidth: 100; implicitHeight: root.height
        radius: root.radius
        color: Style.listViewBkgHighlightColor//Style.brightWidgetBorderColor
        //Q.Behavior on x { Q.NumberAnimation { duration: 300; easing.type: Q.Easing.InOutQuad } }
        Q.Behavior on x {
            enabled: false
            Q.Component.onCompleted: enabled = true
            Q.SmoothedAnimation { velocity: 400; easing.type: Q.Easing.InOutQuad }
        }
    }
    RowLayout {
        id: layout
        spacing: 0
    }
    onModelChanged: {
        for (let child of layout.children) child.destroy()
        for (let text of model) {
            button.createObject(layout, {"text": text});
        }
    }
}
