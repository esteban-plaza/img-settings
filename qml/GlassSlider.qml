import QtQuick
import QtQuick.Controls
import QtQuick.Effects

Slider {
    id: control
    implicitWidth: 140
    implicitHeight: 24
    opacity: enabled ? 1 : 0.45

    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 5
        radius: 2.5
        color: Theme.trackOff

        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: 2.5
            color: Theme.accent
        }
    }

    handle: Rectangle {
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: 26
        height: 17
        radius: height / 2
        scale: control.pressed ? 1.35 : 1
        color: control.pressed ? Qt.rgba(1, 1, 1, 0.55) : "#FFFFFF"
        border.width: control.pressed ? 1 : 0
        border.color: "#E6FFFFFF"
        Behavior on scale { NumberAnimation { duration: 220; easing.type: Easing.OutBack } }
        Behavior on color { ColorAnimation { duration: 160 } }

        layer.enabled: true
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowOpacity: 0.3
            shadowBlur: 0.3
            shadowVerticalOffset: 1
        }
    }
}
