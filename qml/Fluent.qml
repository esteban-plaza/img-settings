pragma Singleton
import QtQuick

// Windows 11 Fluent (WinUI 3) design tokens, light + dark.
QtObject {
    id: fluent

    readonly property bool dark: Application.styleHints.colorScheme === Qt.ColorScheme.Dark

    function pick(light, dark) { return fluent.dark ? dark : light }

    // The user's Windows accent color; Fluent uses a darker shade in light
    // mode and a lighter one in dark mode (AccentDark1 / AccentLight2).
    property color systemAccent: "#0078D4"
    readonly property color accent: dark
        ? Qt.hsla(systemAccent.hslHue, systemAccent.hslSaturation, Math.max(systemAccent.hslLightness, 0.69), 1)
        : Qt.hsla(systemAccent.hslHue, systemAccent.hslSaturation, Math.min(systemAccent.hslLightness, 0.36), 1)

    // Surfaces
    readonly property color base: pick("#F3F3F3", "#202020")            // Mica fallback
    readonly property color card: pick("#B3FFFFFF", "#0DFFFFFF")
    readonly property color cardStroke: pick("#0F000000", "#19000000")
    readonly property color subtleFill: pick("#09000000", "#0FFFFFFF")
    readonly property color divider: pick("#14000000", "#15FFFFFF")

    // Text
    readonly property color textPrimary: pick("#E4000000", "#FFFFFF")
    readonly property color textSecondary: pick("#9E000000", "#C8FFFFFF")
    readonly property color textTertiary: pick("#72000000", "#8BFFFFFF")

    // Status
    readonly property color critical: pick("#C42B1C", "#FF99A4")
    readonly property color criticalBackground: pick("#FDE7E9", "#442726")
    readonly property color caution: pick("#9D5D00", "#FCE100")
    readonly property color cautionBackground: pick("#FFF4CE", "#433519")

    // Geometry
    readonly property int controlRadius: 4
    readonly property int overlayRadius: 8

    // Type ramp
    readonly property int caption: 12
    readonly property int body: 14
    readonly property int subtitle: 20
    readonly property int title: 28

    // Motion: short, decelerating, no overshoot
    readonly property int fast: 167
    readonly property int normal: 250
}
