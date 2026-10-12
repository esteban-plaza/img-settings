import QtQuick
import QtQuick.Effects

// A Liquid Glass surface. It refracts `backdrop`, so it must be a sibling of
// the backdrop item (same coordinate system).
Item {
    id: root
    property Item backdrop
    property real radius: 26
    property color tintColor: Theme.glassTint
    property real tintAmount: Theme.glassTintAmount
    property real refraction: 12
    property real edgeWidth: Math.min(22, radius)
    property real rimStrength: Theme.glassRim
    property real blurAmount: 1.0
    property real shadowOpacity: Theme.dark ? 0.45 : 0.14
    default property alias content: body.data
    readonly property real pad: 16

    RectangularShadow {
        anchors.fill: parent
        radius: root.radius
        offset.y: 10
        blur: 40
        spread: -6
        color: Qt.rgba(0, 0, 0, root.shadowOpacity)
    }

    ShaderEffectSource {
        id: grab
        sourceItem: root.backdrop
        sourceRect: Qt.rect(root.x - root.pad, root.y - root.pad,
                            root.width + 2 * root.pad, root.height + 2 * root.pad)
        width: root.width + 2 * root.pad
        height: root.height + 2 * root.pad
        visible: false
    }

    MultiEffect {
        id: blurred
        source: grab
        width: grab.width
        height: grab.height
        autoPaddingEnabled: false
        blurEnabled: true
        blur: root.blurAmount
        blurMax: 40
        saturation: 0.6
        visible: false
        layer.enabled: true
    }

    ShaderEffect {
        anchors.fill: parent
        property variant source: blurred
        property vector2d itemSize: Qt.vector2d(width, height)
        property vector2d texSize: Qt.vector2d(blurred.width, blurred.height)
        property real pad: root.pad
        property real radius: root.radius
        property real refraction: root.refraction
        property real edgeWidth: root.edgeWidth
        property real rimStrength: root.rimStrength
        property vector4d tint: Qt.vector4d(root.tintColor.r, root.tintColor.g, root.tintColor.b, root.tintAmount)
        fragmentShader: "qrc:/shaders/glass.frag.qsb"
    }

    Item {
        id: body
        anchors.fill: parent
    }
}
