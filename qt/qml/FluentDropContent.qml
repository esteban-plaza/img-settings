import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

Item {
    id: root
    property bool dragActive: false
    property string formatList
    signal browseFolder()
    signal browseFiles()

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.browseFolder()
    }

    // Dashed drop target
    Shape {
        anchors.fill: parent
        anchors.margins: 12
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: root.dragActive ? Fluent.accent : Fluent.textTertiary
            strokeWidth: root.dragActive ? 2 : 1
            strokeStyle: root.dragActive ? ShapePath.SolidLine : ShapePath.DashLine
            dashPattern: [5, 4]
            fillColor: root.dragActive ? Qt.rgba(Fluent.accent.r, Fluent.accent.g, Fluent.accent.b, 0.08) : "transparent"
            PathRectangle {
                x: 0
                y: 0
                width: root.width - 24
                height: root.height - 24
                radius: Fluent.overlayRadius
            }
        }
    }

    Column {
        anchors.centerIn: parent
        spacing: 8

        Icon {
            anchors.horizontalCenter: parent.horizontalCenter
            path: Icons.photos
            size: 48
            strokeWidth: 1.4
            color: root.dragActive ? Fluent.accent : Fluent.textSecondary
        }

        Item { width: 1; height: 4 }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.dragActive ? qsTr("Drop to add watermarks") : qsTr("Drag photos or folders here")
            color: Fluent.textPrimary
            font.pixelSize: Fluent.subtitle
            font.weight: Font.DemiBold
        }

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Supports %1. Results are saved as JPG, ready for WhatsApp HD.").arg(root.formatList)
            color: Fluent.textSecondary
            font.pixelSize: Fluent.body
        }

        Item { width: 1; height: 12 }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            Button {
                highlighted: true
                text: qsTr("Choose folder…")
                onClicked: root.browseFolder()
            }
            Button {
                text: qsTr("Choose photos…")
                onClicked: root.browseFiles()
            }
        }
    }
}
