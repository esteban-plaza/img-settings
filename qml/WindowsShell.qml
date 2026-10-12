import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Windows 11 look: Fluent (WinUI 3) page on a Mica-toned base, with the
// FluentWinUI3 Qt Quick Controls style providing the native controls.
Item {
    id: shell
    required property var app

    function notify(message) { infoBar.show(message) }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        anchors.topMargin: 20
        spacing: 16

        ColumnLayout {
            spacing: 2
            Text {
                text: "img-settings"
                color: Fluent.textPrimary
                font.pixelSize: Fluent.title
                font.weight: Font.DemiBold
            }
            Text {
                text: qsTr("Stamp your camera settings on your photos, ready to share.")
                color: Fluent.textSecondary
                font.pixelSize: Fluent.body
            }
        }

        InfoBar {
            id: infoBar
            Layout.fillWidth: true
        }

        // Main card
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: Fluent.overlayRadius
            color: Fluent.card
            border.width: 1
            border.color: Fluent.cardStroke

            FluentDropContent {
                anchors.fill: parent
                visible: !shell.app.showResults
                dragActive: shell.app.dragActive
                formatList: shell.app.formatList
                onBrowseFolder: shell.app.browseFolder()
                onBrowseFiles: shell.app.browseFiles()
            }

            FluentResults {
                anchors.fill: parent
                visible: shell.app.showResults
                processor: shell.app.processor
                results: shell.app.results
                cancelled: shell.app.wasCancelled
                onProcessMore: shell.app.reset()
            }
        }

        // Options card
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 64
            radius: Fluent.overlayRadius
            color: Fluent.card
            border.width: 1
            border.color: Fluent.cardStroke
            enabled: !shell.app.processor.busy
            opacity: enabled ? 1 : 0.6

            component Divider: Rectangle {
                Layout.preferredWidth: 1
                Layout.preferredHeight: 32
                color: Fluent.divider
            }
            component Label: Text {
                color: Fluent.textPrimary
                font.pixelSize: Fluent.body
            }

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                spacing: 16

                Switch {
                    text: qsTr("Subfolders")
                    checked: shell.app.recurse
                    onToggled: shell.app.recurse = checked
                }

                Divider { }

                Label { text: qsTr("Opacity") }
                Slider {
                    Layout.preferredWidth: 150
                    from: 0
                    to: 1
                    value: shell.app.watermarkOpacity
                    onMoved: shell.app.watermarkOpacity = value
                }
                Label {
                    Layout.preferredWidth: 36
                    text: Math.round(shell.app.watermarkOpacity * 100) + "%"
                    color: Fluent.textSecondary
                    horizontalAlignment: Text.AlignRight
                }

                Divider { }

                Label { text: qsTr("Save to") }
                Button {
                    Layout.maximumWidth: 260
                    text: shell.app.outLabel
                    onClicked: shell.app.chooseOutput()
                }
                Button {
                    visible: shell.app.customOutDir.length > 0
                    flat: true
                    text: qsTr("Reset")
                    onClicked: shell.app.customOutDir = ""
                }

                Item { Layout.fillWidth: true }
            }
        }
    }
}
