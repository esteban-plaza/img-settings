import QtQuick

Item {
    id: root
    property bool dragActive: false
    property var formats: []
    signal browseFolder()
    signal browseFiles()

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.browseFolder()
    }

    // Accent glow while something is dragged over the window.
    Rectangle {
        anchors.fill: parent
        anchors.margins: 10
        radius: 22
        color: "transparent"
        border.width: 2
        border.color: Theme.accent
        opacity: root.dragActive ? 0.9 : 0
        Behavior on opacity { NumberAnimation { duration: 180 } }
    }

    Column {
        anchors.centerIn: parent
        spacing: 12

        Item {
            anchors.horizontalCenter: parent.horizontalCenter
            width: 92
            height: 92
            scale: root.dragActive ? 1.15 : 1
            Behavior on scale { NumberAnimation { duration: Theme.spring; easing.type: Easing.OutBack; easing.overshoot: 2 } }

            Rectangle {
                anchors.fill: parent
                radius: width / 2
                color: Theme.controlFill
                border.width: 1
                border.color: Theme.controlBorder
            }
            Icon {
                anchors.centerIn: parent
                path: Icons.photos
                size: 50
                strokeWidth: 1.5
                color: root.dragActive ? Theme.accent : Theme.secondaryLabel
                Behavior on color { ColorAnimation { duration: 180 } }
            }
        }

        Item { width: 1; height: 4 }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.dragActive ? qsTr("Release to add watermarks") : qsTr("Drop photos or folders here")
            color: Theme.label
            font.pixelSize: 22
            font.weight: Font.DemiBold
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("or click to choose a folder")
            color: Theme.secondaryLabel
            font.pixelSize: 14
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 6
            topPadding: 4
            Repeater {
                model: root.formats
                delegate: Rectangle {
                    required property string modelData
                    width: chip.implicitWidth + 16
                    height: 20
                    radius: 10
                    color: Theme.controlFill
                    Text {
                        id: chip
                        anchors.centerIn: parent
                        text: parent.modelData
                        color: Theme.secondaryLabel
                        font.pixelSize: 10
                        font.weight: Font.DemiBold
                        font.letterSpacing: 0.6
                    }
                }
            }
        }

        Item { width: 1; height: 8 }

        GlassButton {
            anchors.horizontalCenter: parent.horizontalCenter
            icon: Icons.plus
            text: qsTr("Choose Photos…")
            onClicked: root.browseFiles()
        }
    }
}
