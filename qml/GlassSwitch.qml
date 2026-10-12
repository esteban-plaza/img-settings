import QtQuick
import QtQuick.Effects

// Controlled toggle: the owner updates `checked` in response to `toggled()`.
// While pressed the knob swells into a clear glass lens, as in iOS 26.
Item {
    id: root
    property bool checked: false
    signal toggled()

    implicitWidth: 42
    implicitHeight: 24
    opacity: enabled ? 1 : 0.45

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? Theme.green : Theme.trackOff
        Behavior on color { ColorAnimation { duration: 200 } }
    }

    Rectangle {
        id: knob
        y: 2
        height: root.height - 4
        width: mouse.pressed ? height + 10 : height
        x: root.checked ? root.width - width - 2 : 2
        radius: height / 2
        scale: mouse.pressed ? 1.3 : 1
        color: mouse.pressed ? Qt.rgba(1, 1, 1, 0.55) : "#FFFFFF"
        border.width: mouse.pressed ? 1 : 0
        border.color: "#E6FFFFFF"
        Behavior on x { NumberAnimation { duration: Theme.spring; easing.type: Easing.OutBack; easing.overshoot: 1.4 } }
        Behavior on width { NumberAnimation { duration: 180 } }
        Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutBack } }
        Behavior on color { ColorAnimation { duration: 160 } }

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowOpacity: 0.28
            shadowBlur: 0.3
            shadowVerticalOffset: 1
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggled()
    }
}
