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
    transform: Scale {
        id: entryScale
        origin.x: root.width / 2
        origin.y: root.height / 2
        xScale: root.fresh ? 0.85 : 1
        yScale: xScale
    }

    Component.onCompleted: if (fresh) entry.start()

    ParallelAnimation {
        id: entry
        NumberAnimation { target: root; property: "opacity"; to: 1; duration: 200 }
        NumberAnimation { target: entryScale; property: "xScale"; to: 1; duration: 320; easing.type: Easing.OutBack }
    }

    Item {
        id: card
        anchors.fill: parent
        anchors.margins: 7
        scale: hover.hovered && !root.failed ? 1.04 : 1
        Behavior on scale { NumberAnimation { duration: Theme.spring; easing.type: Easing.OutBack; easing.overshoot: 1.8 } }

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
            width: parent.width
            height: parent.height - 36

            Rectangle {
                anchors.fill: parent
                radius: 14
                color: root.failed ? Qt.rgba(Theme.red.r, Theme.red.g, Theme.red.b, 0.14) : Theme.controlFill
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
                Rectangle { anchors.fill: parent; radius: 14 }
            }

            MultiEffect {
                anchors.fill: parent
                source: thumb
                maskEnabled: true
                maskSource: mask
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
                opacity: thumb.status === Image.Ready ? 1 : 0
                Behavior on opacity { NumberAnimation { duration: 300 } }
            }

            // Glass rim on the thumbnail
            Rectangle {
                anchors.fill: parent
                radius: 14
                color: "transparent"
                border.width: 1
                border.color: Theme.controlBorder
            }

            Rectangle {
                visible: root.failed
                anchors.centerIn: parent
                width: 40
                height: 40
                radius: 20
                color: Theme.red
                Icon {
                    anchors.centerIn: parent
                    path: Icons.exclamation
                    size: 24
                    strokeWidth: 2.4
                    color: "#FFFFFF"
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                width: 28
                height: 28
                running: !root.failed && thumb.status === Image.Loading
                visible: running
            }
        }

        Column {
            anchors.top: frame.bottom
            anchors.topMargin: 6
            width: parent.width
            spacing: 1

            Text {
                width: parent.width
                text: root.name
                elide: Text.ElideMiddle
                color: Theme.label
                font.pixelSize: 12
                font.weight: Font.Medium
            }
            Text {
                width: parent.width
                text: root.failed ? root.error : (root.bytes / 1e6).toFixed(1) + " MB"
                elide: Text.ElideRight
                color: root.failed ? Theme.red : Theme.secondaryLabel
                font.pixelSize: 11
            }
        }
    }
}
