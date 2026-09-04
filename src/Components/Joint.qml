import QtQuick

QtObject {
    property real angle: 0
    property real virtualAngle: 0
    property real min: -180
    property real max: 180
    function setAngle(new_angle) {
        if (new_angle < min) new_angle = min
        if (new_angle > max) new_angle = max
        angle = new_angle
    }
}
