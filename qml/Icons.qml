pragma Singleton
import QtQuick

// SF Symbols–like glyphs as SVG path data on a 24×24 grid.
QtObject {
    readonly property string photos: "M4 9a2 2 0 0 1 2-2h11a2 2 0 0 1 2 2v9a2 2 0 0 1-2 2H6a2 2 0 0 1-2-2z"
        + "M7.5 4h11A2.5 2.5 0 0 1 21 6.5V15"
        + "M4.5 17l4-4 3.5 3.5 2-2 4.5 4.5"
        + "M13.2 11a1.3 1.3 0 1 0 2.6 0a1.3 1.3 0 1 0-2.6 0z"
    readonly property string folder: "M3 7.5a2 2 0 0 1 2-2h4l2 2h8a2 2 0 0 1 2 2v8a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"
    readonly property string layers: "M12 3.5l8.5 4.5-8.5 4.5-8.5-4.5z M3.5 12.5l8.5 4.5 8.5-4.5 M3.5 16.5l8.5 4.5 8.5-4.5"
    readonly property string drop: "M12 3.5c3.2 4 6 7.2 6 10.5a6 6 0 0 1-12 0c0-3.3 2.8-6.5 6-10.5z"
    readonly property string xmark: "M7 7l10 10M17 7L7 17"
    readonly property string check: "M5.5 12.5l4 4 9-9"
    readonly property string plus: "M12 5.5v13M5.5 12h13"
    readonly property string exclamation: "M12 6.5v7M12 17.5v.01"
    readonly property string stop: "M8 8h8v8H8z"
}
