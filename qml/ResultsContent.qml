import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var processor
    property ListModel results
    property bool cancelled: false
    signal processMore()

    readonly property bool running: processor.busy
    readonly property real progress: processor.total > 0 ? processor.completed / processor.total : 0

    Column {
        id: header
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 24
        spacing: 14

        Item {
            width: parent.width
            height: titles.implicitHeight

            Column {
                id: titles
                anchors.left: parent.left
                anchors.right: stopButton.left
                anchors.rightMargin: 12
                spacing: 3

                Text {
                    text: root.running ? qsTr("Adding watermarks…")
                        : root.cancelled ? qsTr("Stopped")
                        : root.processor.failed > 0 ? qsTr("Done, with some problems")
                        : qsTr("All done")
                    color: Theme.label
                    font.pixelSize: 20
                    font.weight: Font.DemiBold
                }
                Text {
                    width: parent.width
                    elide: Text.ElideMiddle
                    color: Theme.secondaryLabel
                    font.pixelSize: 12
                    text: {
                        let s = qsTr("%1 of %2 photos").arg(root.processor.completed).arg(root.processor.total)
                        if (root.processor.failed > 0)
                            s += " · " + qsTr("%1 failed").arg(root.processor.failed)
                        return s + "  →  " + root.processor.displayName(root.processor.outDir)
                    }
                }
            }

            GlassButton {
                id: stopButton
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                visible: root.running
                icon: Icons.stop
                text: qsTr("Stop")
                onClicked: root.processor.cancel()
            }
        }

        // Progress capsule
        Rectangle {
            width: parent.width
            height: 6
            radius: 3
            color: Theme.trackOff

            Rectangle {
                width: Math.max(parent.height, parent.width * root.progress)
                height: parent.height
                radius: 3
                color: root.processor.failed > 0 && !root.running ? Theme.red : Theme.accent
                Behavior on width { NumberAnimation { duration: 250; easing.type: Easing.OutCubic } }
            }
        }
    }

    GridView {
        id: grid
        anchors.top: header.bottom
        anchors.topMargin: 12
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footer.top
        anchors.leftMargin: 17
        anchors.rightMargin: 17
        clip: true
        model: root.results
        cellWidth: width / Math.max(2, Math.floor(width / 180))
        cellHeight: cellWidth * 0.75 + 36
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar { }


        // Tiles animate their own entrance (see ResultTile); the view only slides
        // existing tiles aside when a new one is inserted before them.
        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 260; easing.type: Easing.OutCubic }
        }

        delegate: ResultTile {
            width: grid.cellWidth
            height: grid.cellHeight
            onOpen: root.processor.openUrl(outputUrl)
        }
    }

    Row {
        id: footer
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.running ? -height : 20
        height: root.running ? 0 : 34
        spacing: 10
        opacity: root.running ? 0 : 1
        Behavior on opacity { NumberAnimation { duration: 250 } }

        GlassButton {
            prominent: true
            icon: Icons.folder
            text: Qt.platform.os === "windows" ? qsTr("Open in Explorer") : qsTr("Reveal in Finder")
            onClicked: root.processor.reveal(root.processor.outDir)
        }
        GlassButton {
            icon: Icons.plus
            text: qsTr("Process More")
            onClicked: root.processMore()
        }
    }
}
