import QtQuick
// import QtQuick.Shapes
import QtLocation
import QtPositioning
import QtQuick.Layouts
import Components as C
import QmlComponents
import App

Window {
    width: 960
    height: 640
    visible: true
    title: "Hobot Slasher"
    color: "#212830"

    Plugin {
        id: mapPlugin
        name: "yandex"
        locales: ["ru_RU"]

        PluginParameter {
            name: "yandex.mapping.highdpi_tiles"
            value: "false"
        }
        PluginParameter {
            name: "yandex.mapping.cache.directory"
            value: "/tmp/yandex_cache"
        }

        // name: "osm"
        // PluginParameter {
        //     name: "osm.mapping.host"
        //     value: "https://tile.openstreetmap.org"
        // }
        // PluginParameter {
        //     name: "osm.mapping.providersrepository.disabled"
        //     value: "true"
        // }
        // PluginParameter {
        //     name: "osm.user_agent"
        //     value: "MyNavigator/1.0"
        // }
    }
    // Image {
    //     scale: 1 / Screen.devicePixelRatio / 1.5
    //     source: "qrc:/images/yandex_sat_100m.png"
    //     anchors.centerIn: map_view
    // }

    // Карта
    MapViewer {
        id: map_view
        width: parent.width - control_panel.width; height: parent.height
        // width: parent.width; height: parent.height
        map {
            //visible: false
            //opacity: 0.5
            plugin: mapPlugin
            zoomLevel: 18 // 15
            copyrightsVisible: false
            //center: QtPositioning.coordinate(55.751244, 37.618423) // Москва по умолчанию
            // 18/56.29816/38.19979
            center: QtPositioning.coordinate(56.29816, 38.19979)
        }
        PointHandler {
            parent: map_view.map
            onActiveChanged: {
                if (active) {
                    var pos = map_view.map.toCoordinate(point.position)
                    if (set_area_button.checked) {
                        // console.log(pos.latitude, pos.longitude)
                        // var mp = map_point.createObject(map, { coordinate: pos })
                        // map.addMapItem(mp)
                        // working_area.path.push({ latitude: pos.latitude, longitude: pos.longitude })
                        working_area.addCoordinate({ latitude: pos.latitude, longitude: pos.longitude })
                    } else {
                        var points = []
                        for (let coord of working_area.path) {
                            points.push(Qt.point(coord.latitude, coord.longitude))
                        }
                        console.log("contains:", App.interface.areaContainsPoint(Qt.point(pos.latitude, pos.longitude), points))
                    }
                }
            }
        }

        MapQuickItem {
            id: marker
            parent: map_view.map
            // Задаем координаты точки (широта, долгота) [citation:1]
            //coordinate: QtPositioning.coordinate(55.751244, 37.618423)
            coordinate: QtPositioning.coordinate(56.29816, 38.19979)

            // Смещение точки привязки (центрируем маркер) [citation:1]
            anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)

            // Визуальное представление маркера
            // sourceItem: Rectangle {
            //     width: 20
            //     height: 20
            //     radius: 10
            //     color: "red"
            //     border.width: 2
            //     border.color: "white"
            // }
            sourceItem: Image {
                source: "qrc:/images/map_arrow.svg"
                width: 24; height: 28.8

                // sourceSize: Qt.size(width, height)
                // fillMode: Image.PreserveAspectFit
                mipmap: true
                verticalAlignment: Qt.AlignVCenter
                horizontalAlignment: Qt.AlignHCenter
            }
        }
        MapPolygon {
            id: working_area
            parent: map_view.map
            color: "#5500FF00"
            border.color: "#006600"
            border.width: 2
            layer.enabled: true
            layer.smooth: true
            // referenceSurface: QtLocation.ReferenceSurface.Map
        }
        /*
        Rectangle {
            id: marker_2
            Component.onCompleted: {
                console.log(map_view.map.fromCoordinate(QtPositioning.coordinate(56.29816, 38.19979)).x,
                            map_view.map.fromCoordinate(QtPositioning.coordinate(56.29816, 38.19979)).y)

                x = map_view.map.fromCoordinate(QtPositioning.coordinate(56.29816, 38.19979)).x / Screen.devicePixelRatio - 6
                y = map_view.map.fromCoordinate(QtPositioning.coordinate(56.29816, 38.19979)).y / Screen.devicePixelRatio - 6
            }
            width: 12
            height: 12
            radius: 6
            color: "red"
            border.width: 1.5
            border.color: "white"
        }
        */
        Component.onCompleted: {
            //map.clearData()
            for (let mapType of map.supportedMapTypes) {
                console.log("=======================")
                console.log("name:", mapType.name)
                console.log("description:", mapType.description)
                console.log("style:", mapType.style)
                // if (mapType.name === "Hybrid Map") {
                //     map.activeMapType = mapType
                // }
            }
        }
    }
    C.Flickable {
        // visible: false
        id: control_panel
        clip: true
        x: map_view.width
        width: control_panel_layout.width + 24; height: parent.height
        contentWidth: width; contentHeight: control_panel_layout.height
        boundsMovement: Flickable.StopAtBounds
        boundsBehavior: Flickable.DragAndOvershootBounds
        component CPText: C.Text {
            horizontalAlignment: Qt.AlignRight
            Layout.alignment: Qt.AlignVCenter
        }
        ColumnLayout {
            id: control_panel_layout
            spacing: 12
            anchors.centerIn: parent
            //width: implicitWidth < control_panel.width ? control_panel.width : implicitWidth
            height: implicitHeight < control_panel.height ? control_panel.height : implicitHeight
            GridLayout {
                id: coordinates
                rows: 3
                flow: GridLayout.TopToBottom
                rowSpacing: 16; columnSpacing: 16
                Layout.alignment: Qt.AlignCenter
                CPText { text: "X, м" }
                CPText { text: "Y, м" }
                CPText { text: "yaw, °" }
                C.NumberField {
                    id: coord_x
                    decimals: 4
                    min: -Infinity; max: Infinity;
                    // onValueModified: App.device.protocol.setX(value / 1000)
                }
                C.NumberField {
                    id: coord_y
                    decimals: 4
                    min: -Infinity; max: Infinity;
                    // onValueModified: App.device.protocol.setY(value / 1000)
                }
                C.NumberField {
                    id: yaw_field
                    decimals: 4
                    min: -Infinity; max: Infinity;
                    // onValueModified: App.device.protocol.setZ(value / 1000)
                }
                CPText { text: "vx, м/с"; Layout.preferredWidth: yaw_label.width }
                CPText { text: "vy, м/с"; Layout.preferredWidth: yaw_label.width }
                CPText { id: yaw_label; text: "v yaw, °/с" }
                C.NumberField {
                    id: velocity_x
                    decimals: 4
                    min: -360; max: 360
                }
                C.NumberField {
                    id: velocity_y
                    decimals: 4
                    min: -360; max: 360
                }
                C.NumberField {
                    id: velocity_yaw
                    decimals: 4
                    min: -360; max: 360
                    // onValueModified: App.interface.setYaw(value * Math.PI / 180)
                }
            }
            C.Joystick {
                id: joystick_1
                toX: 1000; toY: 1000
                startX: 500; startY: 0
                Layout.alignment: Qt.AlignHCenter
            }
            C.Joystick {
                visible: false
                id: joystick_2
                toX: 1000; toY: 1000
                startX: 500; startY: 500
                Layout.alignment: Qt.AlignHCenter
            }
            C.ButtonJoystick {
                id: buttonJoystick
                //leftText: "🡰"; rightText: "🡲"; topText: "🡱"; bottomText: "🡳"
                leftText: "◄"; rightText: "►"; topText: "▲"; bottomText: "▼"
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
    C.Button {
        id: set_area_button
        text: checked ? "Сохранить" : "Изменить границы"
        x: 15; y: 15
        width: 150; height: 30
        checkable: true
        onToggled: {
            if (checked) {
                working_area.path = []
            }
        }
    }
    C.Button {
        id: mowing_button
        text: checked ? "Завершить покос" : "Начать покос"
        width: 150; height: 30
        anchors.left: set_area_button.left; anchors.top: set_area_button.bottom; anchors.topMargin: 15
        checkable: true
        onToggled: App.interface.setMowingEnabled(checked)
    }
    // Component {
    //     id: map_point
    //     MapQuickItem {
    //         // coordinate: QP.QtPositioning.coordinate(56.29816, 38.19979)
    //         anchorPoint: Qt.point(sourceItem.width / 2, sourceItem.height / 2)
    //         sourceItem: Rectangle {
    //             width: 10
    //             height: 10
    //             radius: 5
    //             color: "green"
    //             border.width: 1
    //             border.color: "white"
    //         }
    //     }
    // }
    Timer {
        interval: 100
        running: true
        repeat: true
        onTriggered: {
            var pwm = 90 * 0.2
            if (joystick_1.active) {
                App.interface.setMoveParams(joystick_1.valueX, joystick_1.valueY, joystick_2.valueX, joystick_2.valueY);
            } else if (joystick_2.active) {
                App.interface.setMoveParams(joystick_1.valueX, joystick_1.valueY, joystick_2.valueX, joystick_2.valueY);
            } else if (buttonJoystick.topStatePressed) {
                App.interface.setMotorSpeeds(90 + 90 * 0.25, 90 + 90 * 0.2)
                // console.log("forward")
            } else if (buttonJoystick.bottomStatePressed) {
                App.interface.setMotorSpeeds(90 - 90 * 0.2, 90 - 90 * 0.28)
            } else if (buttonJoystick.leftStatePressed) {
                App.interface.setMotorSpeeds(90 - 90 * 0.06, 90 + 90 * 0.06)
            } else if (buttonJoystick.rightStatePressed) {
                App.interface.setMotorSpeeds(90 + 90 * 0.1, 90 - 90 * 0.1)
            } else {
                App.interface.setMotorSpeeds(90, 90)
                // console.log("stop")
            }
        }
    }
    Connections {
        target: App.interface
        ignoreUnknownSignals: true
        function onLocalPositionChanged(x: real, y: real, yaw: real) {
            coord_x.setValue(x)
            coord_y.setValue(y)
            yaw_field.setValue(yaw * 180 / Math.PI)
        }
        function onVelocitiesChanged(vx: real, vy: real, v_yaw: real) {
            velocity_x.setValue(vx)
            velocity_y.setValue(vy)
            velocity_yaw.setValue(v_yaw * 180 / Math.PI)
        }
        function onGlobalPositionChanged(latitude: real, longitude: real, altitude: int) {
            marker.coordinate = QtPositioning.coordinate(latitude, longitude, altitude)
            // var coord = map_view.map.fromCoordinate(QtPositioning.coordinate(latitude, longitude, altitude))
            // marker_2.x = coord.x / Screen.devicePixelRatio - 6
            // marker_2.y = coord.y / Screen.devicePixelRatio - 6
        }
        function onYawChanged(yaw: real) {
            marker.rotation = yaw
        }
    }

    Window {
        title: "grid map viewer"
        width: 800; height: 600
        visible: true
        // color: C.Style.backgroundColor_1
        transientParent: null
        LidarMapViewer {
            id: lidar_map_viewer
            width: parent.width; height: parent.height
            trajectoryVisible: trajectory_radio_button.checked
            relative: true
            C.RadioButton {
                id: trajectory_radio_button
                text: "Показывать траекторию"
                font.pointSize: 10
                x: 15; y: 15
                height: 30
                checked: true
            }
            C.Button {
                id: build_map_button
                text: checked ? "Завершить сканирование" : "Начать сканирование"
                width: 150; height: 30
                anchors.left: trajectory_radio_button.left; anchors.top: trajectory_radio_button.bottom; anchors.topMargin: 15
                checkable: true
                onToggled: {
                    if (checked) {
                        App.interface.startBuildingMap()
                        lidar_map_viewer.clearTrajectory()
                    } else {
                        App.interface.finishBuildingMap()
                    }
                }
            }
            C.Button {
                id: trajectory_clear_button
                text: "Очистить траекторию"
                width: 150; height: 30
                anchors.left: build_map_button.left; anchors.top: build_map_button.bottom; anchors.topMargin: 15
                onClicked: lidar_map_viewer.clearTrajectory()
            }
            C.Button {
                id: relative_button
                text: "Сменить ориентацию"
                width: 150; height: 30
                anchors.left: trajectory_clear_button.left; anchors.top: trajectory_clear_button.bottom; anchors.topMargin: 15
                onClicked: lidar_map_viewer.relative = !lidar_map_viewer.relative
            }
            WheelHandler {
                onWheel: (event) => {
                             lidar_map_viewer.zoom += event.angleDelta.y / 1200
                         }
            }
        }
    }
    Window {
        title: "lidar viewer"
        width: 800
        height: 600
        visible: true
        transientParent: null
        LidarViewer {
            id: lidar_viewer
            width: parent.width; height: parent.height
            WheelHandler {
                onWheel: (event) => {
                             lidar_viewer.zoom += event.angleDelta.y / 1200
                         }
            }
        }
    }
}
