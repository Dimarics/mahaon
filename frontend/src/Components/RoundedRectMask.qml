import QtQuick
import QtQuick.Shapes

Shape {
    property int radius
    containsMode: Shape.FillContains
    ShapePath {
        startY: radius
        PathArc { x: radius; radiusX: radius; radiusY: radius}
        PathLine { x: width - radius }
        PathArc { x: width; y: radius; radiusX: radius; radiusY: radius}
        PathLine { x: width; y: height - radius }
        PathArc { x: width - radius; y: height; radiusX: radius; radiusY: radius }
        PathLine { x: radius; y: height }
        PathArc { y: height - radius; radiusX: radius; radiusY: radius }
    }
}
