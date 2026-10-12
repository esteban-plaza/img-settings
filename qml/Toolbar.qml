import QtQuick

// Floating glass capsule with the processing options.
GlassPanel {
    id: root
    property bool recurse: false
    property real watermarkOpacity: 0.82
    property string outLabel
    property bool customOutput: false
    signal recurseToggled()
    signal opacityMoved(real value)
    signal chooseOutput()
    signal clearOutput()

    width: row.implicitWidth + 36
    height: 54
    radius: height / 2
    edgeWidth: 16
    opacity: enabled ? 1 : 0.6
    Behavior on opacity { NumberAnimation { duration: 200 } }

    component Label: Text {
        anchors.verticalCenter: parent.verticalCenter
        color: Theme.label
        font.pixelSize: 13
    }
    component Separator: Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        width: 1
        height: 22
        color: Theme.separator
    }

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 16

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Icon { anchors.verticalCenter: parent.verticalCenter; path: Icons.layers; size: 17; color: Theme.secondaryLabel }
            Label { text: qsTr("Subfolders") }
            GlassSwitch {
                anchors.verticalCenter: parent.verticalCenter
                checked: root.recurse
                onToggled: root.recurseToggled()
            }
        }

        Separator { }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Icon { anchors.verticalCenter: parent.verticalCenter; path: Icons.drop; size: 17; color: Theme.secondaryLabel }
            Label { text: qsTr("Opacity") }
            GlassSlider {
                anchors.verticalCenter: parent.verticalCenter
                width: 120
                value: root.watermarkOpacity
                onMoved: root.opacityMoved(value)
            }
            Label {
                width: 34
                text: Math.round(root.watermarkOpacity * 100) + "%"
                color: Theme.secondaryLabel
                horizontalAlignment: Text.AlignRight
                font.features: { "tnum": 1 }
            }
        }

        Separator { }

        Row {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8
            Label { text: qsTr("Output") }
            GlassButton {
                anchors.verticalCenter: parent.verticalCenter
                height: 30
                icon: Icons.folder
                text: root.outLabel
                onClicked: root.chooseOutput()
            }
            GlassButton {
                anchors.verticalCenter: parent.verticalCenter
                visible: root.customOutput
                width: 26
                height: 26
                icon: Icons.xmark
                onClicked: root.clearOutput()
            }
        }
    }
}
