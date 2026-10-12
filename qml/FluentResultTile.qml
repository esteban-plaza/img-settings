import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Item {
    id: root
    required property string name
    required property string outputUrl
    required property real bytes
    required property string error
    required property real addedAt
    signal open()

    readonly property bool failed: error.length > 0
    // Only tiles that just arrived animate in; ones scrolled into view don't.
    readonly property bool fresh: Date.now() - addedAt < 800

    opacity: fresh ? 0 : 1
    transform: Translate {
        id: entryOffset
        y: root.fresh ? 12 : 0
    }

    Component.onCompleted: if (fresh) entry.start()

    ParallelAnimation {
        id: entry
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: Fluent.normal }
        NumberAnimation { target: entryOffset; property: "y"; to: 0; duration: Fluent.normal; easing.type: Easing.OutCubic }
    }

    // Subtle fill on hover/press, as on WinUI grid items.
    Rectangle {
        anchors.fill: parent
        anchors.margins: 2
        radius: Fluent.overlayRadius
        color: Fluent.subtleFill
        opacity: hover.hovered ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: Fluent.fast } }
    }

    HoverHandler {
        id: hover
        cursorShape: root.failed ? Qt.ArrowCursor : Qt.PointingHandCursor
    }
    TapHandler { onTapped: if (!root.failed) root.open() }

    ToolTip.visible: hover.hovered && root.failed
    ToolTip.text: root.error
    ToolTip.delay: 300

    Item {
        id: frame
        x: 8
        y: 8
        width: parent.width - 16
        height: parent.height - 52

        Rectangle {
            anchors.fill: parent
            radius: Fluent.controlRadius
            color: root.failed ? Fluent.criticalBackground : Fluent.subtleFill
            border.width: 1
            border.color: Fluent.cardStroke
        }

        Image {
            id: thumb
            anchors.fill: parent
            visible: false
            source: root.outputUrl
            fillMode: Image.PreserveAspectCrop
            asynchronous: true
            cache: false
            sourceSize: Qt.size(480, 360)
        }

        Item {
            id: mask
            anchors.fill: parent
            visible: false
            layer.enabled: true
            Rectangle { anchors.fill: parent; radius: Fluent.controlRadius }
        }

        MultiEffect {
            anchors.fill: parent
            source: thumb
            maskEnabled: true
            maskSource: mask
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
            opacity: thumb.status === Image.Ready ? 1 : 0
            Behavior on opacity { NumberAnimation { duration: Fluent.normal } }
        }

        // InfoBadge (critical)
        Rectangle {
            visible: root.failed
            anchors.centerIn: parent
            width: 32
            height: 32
            radius: 16
            color: Fluent.critical
            Text {
                anchors.centerIn: parent
                text: "!"
                color: Fluent.dark ? "#000000" : "#FFFFFF"
                font.pixelSize: 18
                font.bold: true
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: !root.failed && thumb.status === Image.Loading
            visible: running
        }
    }

    Column {
        anchors.top: frame.bottom
        anchors.topMargin: 6
        x: 8
        width: parent.width - 16
        spacing: 1

        Text {
            width: parent.width
            text: root.name
            elide: Text.ElideMiddle
            color: Fluent.textPrimary
            font.pixelSize: Fluent.body
        }
        Text {
            width: parent.width
            text: root.failed ? root.error : (root.bytes / 1e6).toFixed(1) + " MB"
            elide: Text.ElideRight
            color: root.failed ? Fluent.critical : Fluent.textSecondary
            font.pixelSize: Fluent.caption
        }
    }
}
