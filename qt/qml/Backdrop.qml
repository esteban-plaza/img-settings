import QtQuick
import QtQuick.Shapes

// What the glass refracts: slow-drifting color fields, and the latest
// processed photo once there is one.
Item {
    id: root
    property url photo
    property real phase: 0
    property bool frontIsA: true

    onPhotoChanged: {
        if (frontIsA)
            photoB.source = photo
        else
            photoA.source = photo
        frontIsA = !frontIsA
    }

    NumberAnimation on phase {
        from: 0
        to: Math.PI * 2
        duration: 40000
        loops: Animation.Infinite
        running: Qt.application.state === Qt.ApplicationActive
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.backdropBase
        Behavior on color { ColorAnimation { duration: 400 } }
    }

    Repeater {
        model: Theme.dark
               ? [{ c: "#2A5BFF", s: 0.95, x: 0.15, y: 0.20 }, { c: "#8E3BE0", s: 0.85, x: 0.85, y: 0.25 },
                  { c: "#E0602A", s: 0.70, x: 0.70, y: 0.90 }, { c: "#109A9A", s: 0.80, x: 0.20, y: 0.85 }]
               : [{ c: "#6FA8FF", s: 0.95, x: 0.15, y: 0.20 }, { c: "#C59BFF", s: 0.85, x: 0.85, y: 0.25 },
                  { c: "#FFB38A", s: 0.70, x: 0.70, y: 0.90 }, { c: "#7FE0D8", s: 0.80, x: 0.20, y: 0.85 }]

        delegate: Shape {
            id: blob
            required property var modelData
            required property int index
            readonly property real d: Math.max(root.width, root.height) * modelData.s
            readonly property real drift: Math.min(root.width, root.height) * 0.12

            width: d
            height: d
            x: root.width * modelData.x - d / 2 + drift * Math.cos(root.phase + index * 1.7)
            y: root.height * modelData.y - d / 2 + drift * Math.sin(root.phase * 2 + index * 2.3)
            opacity: Theme.dark ? 0.75 : 0.85

            ShapePath {
                strokeColor: "transparent"
                fillGradient: RadialGradient {
                    centerX: blob.d / 2; centerY: blob.d / 2; centerRadius: blob.d / 2
                    focalX: blob.d / 2; focalY: blob.d / 2
                    GradientStop { position: 0.0; color: blob.modelData.c }
                    GradientStop { position: 1.0; color: Qt.rgba(Qt.color(blob.modelData.c).r, Qt.color(blob.modelData.c).g, Qt.color(blob.modelData.c).b, 0) }
                }
                startX: 0; startY: 0
                PathLine { x: blob.d; y: 0 }
                PathLine { x: blob.d; y: blob.d }
                PathLine { x: 0; y: blob.d }
                PathLine { x: 0; y: 0 }
            }
        }
    }

    component Photo: Image {
        required property bool front
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        asynchronous: true
        cache: false
        sourceSize: Qt.size(1600, 1600)
        opacity: front && status === Image.Ready && source.toString() !== "" ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 700; easing.type: Easing.InOutQuad } }
    }

    Photo { id: photoA; front: root.frontIsA }
    Photo { id: photoB; front: !root.frontIsA }
}
