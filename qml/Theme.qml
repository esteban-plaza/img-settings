pragma Singleton
import QtQuick

// Apple system colors plus the Liquid Glass material parameters, light + dark.
QtObject {
    id: theme

    readonly property bool dark: Application.styleHints.colorScheme === Qt.ColorScheme.Dark

    function pick(light, dark) { return theme.dark ? dark : light }

    readonly property color accent: pick("#007AFF", "#0A84FF")
    readonly property color green: pick("#34C759", "#30D158")
    readonly property color red: pick("#FF3B30", "#FF453A")

    readonly property color label: pick("#1D1D1F", "#F5F5F7")
    readonly property color secondaryLabel: pick("#4B4B53", "#B4B4BC")
    readonly property color tertiaryLabel: pick("#7C7C85", "#85858E")

    readonly property color backdropBase: pick("#E8ECF4", "#0B0C11")

    // Glass material
    readonly property color glassTint: pick("#FFFFFF", "#1C1C22")
    readonly property real glassTintAmount: pick(0.38, 0.42)
    readonly property real glassRim: pick(0.55, 0.30)

    // Controls sitting on glass
    readonly property color controlFill: pick("#1F000000", "#24FFFFFF")
    readonly property color controlBorder: pick("#40FFFFFF", "#1FFFFFFF")
    readonly property color trackOff: pick("#29000000", "#33FFFFFF")
    readonly property color separator: pick("#1F000000", "#26FFFFFF")

    readonly property int spring: 320
}
