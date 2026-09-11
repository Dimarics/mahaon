import QtQuick

QtObject {
    enum AvailableFeatures {
        Joints, Motors, Coordinates,
        Speed, Accel, Decel,
        Laser, Pomp, Grub, Extruder,
        LineSensor, ColorSensor, TouchSensor, Sonar,
        PWM, RGB, LedStrip, ConveyorBelt
    }
    property string protocolName
    property QtObject protocol: null
    property list<int> availableFeatures
    property list<string> availableProtocols

    property real speed
    property real accel
    property real grub
    property list<Joint> joints

    property Connections connections: Connections {
        target: protocol
        ignoreUnknownSignals: true
        function onJointChanged(id: int, angle: real) {
            joints[id - 1].setAngle(angle)
        }
    }
    function setJoint(id: int, angle: real) {
        if (angle < joints[id - 1].min) angle = joints[id - 1].min
        if (angle > joints[id - 1].max) angle = joints[id - 1].max
        protocol.setJoint(id, angle)
    }
    function rotateMotor(id, dir, speed) { protocol.rotateMotor(id, dir, speed) }
    function stopMotor(id: int) { protocol.stopMotor(id) }
    function setSpeed(value: real) { speed = value; protocol.setSpeed(value) }
    function setAccel(value: real) { accel = value; protocol.setAccel(value) }
    function setGrubValue(value: real) { grub = value; protocol.setGrubValue(value) }
    function stay() { protocol.stay() }
    function stop() { protocol.stop() }
    function goHome() { protocol.goHome() }
}
