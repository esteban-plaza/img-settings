import QtQuick
import QtQuick.Controls

// WinUI InfoBar (warning severity) that collapses when dismissed.
Rectangle {
    id: root
    property string message
    property bool open: false

    function show(text) {
        message = text
        open = true
        dismissTimer.restart()
    }

    implicitHeight: open ? 48 : 0
    visible: implicitHeight > 0
    clip: true
    radius: Fluent.controlRadius
    color: Fluent.cautionBackground
    border.width: 1
    border.color: Fluent.cardStroke
    Behavior on implicitHeight { NumberAnimation { duration: Fluent.normal; easing.type: Easing.OutCubic } }

    Timer {
        id: dismissTimer
        interval: 5000
        onTriggered: root.open = false
    }

    Rectangle {
        id: badge
        x: 16
        y: 24 - height / 2
        width: 16
        height: 16
        radius: 8
        color: Fluent.caution
        Text {
            anchors.centerIn: parent
            text: "!"
            color: Fluent.dark ? "#000000" : "#FFFFFF"
            font.pixelSize: 11
            font.bold: true
        }
    }

    Text {
        anchors.left: badge.right
        anchors.leftMargin: 12
        anchors.right: closeButton.left
        anchors.rightMargin: 8
        y: 24 - height / 2
        text: root.message
        elide: Text.ElideRight
        color: Fluent.textPrimary
        font.pixelSize: Fluent.body
    }

    Button {
        id: closeButton
        anchors.right: parent.right
        anchors.rightMargin: 8
        y: 24 - height / 2
        flat: true
        text: "✕"
        onClicked: root.open = false
    }
}
