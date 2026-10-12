import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root
    property var processor
    property ListModel results
    property bool cancelled: false
    signal processMore()

    readonly property bool running: processor.busy

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: root.running ? qsTr("Adding watermarks…")
                        : root.cancelled ? qsTr("Stopped")
                        : root.processor.failed > 0 ? qsTr("Finished with errors")
                        : qsTr("All photos are ready")
                    color: Fluent.textPrimary
                    font.pixelSize: Fluent.subtitle
                    font.weight: Font.DemiBold
                }
                Text {
                    Layout.fillWidth: true
                    elide: Text.ElideMiddle
                    color: Fluent.textSecondary
                    font.pixelSize: Fluent.body
                    text: {
                        let s = qsTr("%1 of %2 photos").arg(root.processor.completed).arg(root.processor.total)
                        if (root.processor.failed > 0)
                            s += " · " + qsTr("%1 failed").arg(root.processor.failed)
                        return s + " · " + root.processor.displayName(root.processor.outDir)
                    }
                }
            }

            Button {
                visible: root.running
                text: qsTr("Stop")
                onClicked: root.processor.cancel()
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            from: 0
            to: Math.max(1, root.processor.total)
            value: root.processor.completed
        }

        GridView {
            id: grid
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.results
            cellWidth: width / Math.max(2, Math.floor(width / 190))
            cellHeight: cellWidth * 0.72 + 52
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { }

            // Tiles animate their own entrance (see FluentResultTile).
            displaced: Transition {
                NumberAnimation { properties: "x,y"; duration: Fluent.normal; easing.type: Easing.OutCubic }
            }

            delegate: FluentResultTile {
                width: grid.cellWidth
                height: grid.cellHeight
                onOpen: root.processor.openUrl(outputUrl)
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: !root.running
            spacing: 8

            Item { Layout.fillWidth: true }
            Button {
                highlighted: true
                text: Qt.platform.os === "windows" ? qsTr("Open in File Explorer") : qsTr("Reveal in Finder")
                onClicked: root.processor.reveal(root.processor.outDir)
            }
            Button {
                text: qsTr("Process more")
                onClicked: root.processMore()
            }
        }
    }
}
