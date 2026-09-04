import QtQuick as Q
import QtQuick.Layouts

Q.Item {
    id: root
    default property alias extraContent: extraLayout.data
    property bool processRun: true
    //property alias running: startButton.checked
    signal create
    signal open
    signal save
    signal saveAs
    signal clear
    signal startButtonToggled
    signal start
    signal pause
    signal resume
    signal stop
    signal exit
    enum Status { NotRunning, Running, Paused }
    property int status: ToolBar.NotRunning
    //signal startToggled: startButton.toggled
    height: exitButton.height + 30
    Flickable {
        id: flickable
        clip: true
        width: parent.width - exitButton.width - 30; height: parent.height
        contentWidth: Math.max(layout.implicitWidth, flickable.width)
        boundsMovement: width < contentWidth ? Flickable.FollowBoundsBehavior : Flickable.StopAtBounds
        RowLayout {
            id: layout
            spacing: 15
            anchors.fill: parent
            anchors.margins: 15
            Button {
                toolTip: "Создать"
                visible: false
                icon.source: "qrc:/images/create.svg"
                icon.width: 0; icon.height: 40
                onClicked: create()
            }
            Button {
                toolTip: "Открыть"
                icon.source: "qrc:/images/open.svg"
                icon.width: 40; icon.height: 0
                onClicked: open()
            }
            Button {
                toolTip: "Сохранить"
                icon.source: "qrc:/images/save.svg"
                icon.width: 36; icon.height: 36;
                onClicked: save()
            }
            Button {
                visible: Qt.platform.os !== "wasm"
                toolTip: "Сохранить как"
                icon.source: "qrc:/images/save_as.svg"
                icon.width: 36; icon.height: 36;
                onClicked: saveAs()
            }
            Button {
                toolTip: "Очистить"
                icon.source: "qrc:/images/trash.svg"
                icon.width: 32; icon.height: 0;
                onClicked: clear()
            }
            Button {
                id: startButton
                visible: processRun
                toolTip: status === ToolBar.NotRunning ? "Старт" : status === ToolBar.Running ? "Пауза" : "Продолжить"
                //checkable: true
                icon.source: status === ToolBar.Running ? "qrc:/images/pause.svg" : "qrc:/images/start.svg"
                icon.width: 32; icon.height: 32
                /*onToggled: {
                    startButtonToggled()
                    checked ? start() : pause()
                }*/
                onClicked: {
                    switch (status) {
                    case ToolBar.NotRunning:
                        status = ToolBar.Running
                        start()
                        break;
                    case ToolBar.Running:
                        status = ToolBar.Paused
                        pause()
                        break;
                    case ToolBar.Paused:
                        status = ToolBar.Running
                        resume()
                        break;
                    }
                }
            }
            Button {
                visible: processRun
                toolTip: "Стоп"
                icon.source: "qrc:/images/stop.svg"
                icon.width: 32; icon.height: 32
                onClicked: {
                    status = ToolBar.NotRunning
                    stop()
                }
            }
            RowLayout {
                id: extraLayout
                spacing: layout.spacing
                Layout.fillWidth: true
            }
        }
    }
    Button {
        id: exitButton
        toolTip: "Выход"
        icon.source: "qrc:/images/exit.svg"
        icon.width: 32; icon.height: 32
        anchors { verticalCenter: parent.verticalCenter; right: parent.right; margins: 15}
        onClicked: exit()
    }
}
