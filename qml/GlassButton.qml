import QtQuick

// Capsule button. Prominent buttons are filled with the accent color.
Item {
    id: root
    property string text
    property string icon
    property bool prominent: false
    property bool destructive: false
    signal clicked()

    readonly property color foreground: prominent ? "#FFFFFF" : (destructive ? Theme.red : Theme.label)

    implicitHeight: 32
    implicitWidth: text.length > 0 ? content.implicitWidth + 28 : implicitHeight
    opacity: enabled ? 1 : 0.45
    scale: mouse.pressed ? 0.94 : (mouse.containsMouse ? 1.03 : 1)
    Behavior on scale { NumberAnimation { duration: Theme.spring; easing.type: Easing.OutBack; easing.overshoot: 2.2 } }

    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.prominent ? (root.destructive ? Theme.red : Theme.accent) : Theme.controlFill
        border.width: 1
        border.color: root.prominent ? "#40FFFFFF" : Theme.controlBorder

        // Top sheen
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: height / 2
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(1, 1, 1, root.prominent ? 0.30 : 0.16) }
                GradientStop { position: 0.6; color: Qt.rgba(1, 1, 1, 0) }
            }
        }

        Rectangle {
            anchors.fill: parent
            radius: height / 2
            color: "#000000"
            opacity: mouse.pressed ? 0.12 : 0
        }
    }

    Row {
        id: content
        anchors.centerIn: parent
        spacing: 6

        Icon {
            visible: root.icon.length > 0
            anchors.verticalCenter: parent.verticalCenter
            path: root.icon
            size: 16
            color: root.foreground
        }
        Text {
            visible: root.text.length > 0
            anchors.verticalCenter: parent.verticalCenter
            text: root.text
            color: root.foreground
            font.pixelSize: 13
            font.weight: Font.Medium
        }
    }

    MouseArea {
        id: mouse
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }
}
