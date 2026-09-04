import QtQuick
import QmlComponents
import Components
import App

QtObject {
    id: root
    // property QtObject protocol: SerialPortInterface {}
    property QtObject protocol: WebSocketInterface {}
}
