import QtQuick

GlassPanel {
    id: root
    property string text

    function show(message) {
        text = message
        state = "shown"
        hideTimer.restart()
    }

    width: label.implicitWidth + 40
    height: 40
    radius: 20
    edgeWidth: 14
    opacity: 0
    scale: 0.9
    visible: opacity > 0

    states: State {
        name: "shown"
        PropertyChanges { root.opacity: 1; root.scale: 1 }
    }
    transitions: Transition {
        NumberAnimation { properties: "opacity,scale"; duration: Theme.spring; easing.type: Easing.OutBack }
    }

    Timer {
        id: hideTimer
        interval: 3200
        onTriggered: root.state = ""
    }

    Text {
        id: label
        anchors.centerIn: parent
        text: root.text
        color: Theme.label
        font.pixelSize: 13
        font.weight: Font.Medium
    }
}
