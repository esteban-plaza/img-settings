import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import ImgSettings

// Shared app state and actions. The look is delegated to a per-platform shell:
// Liquid Glass on macOS, Fluent (WinUI 3) on Windows.
ApplicationWindow {
    id: window
    required property bool fluent

    property bool showResults: false
    property bool wasCancelled: false
    property bool recurse: false
    property real watermarkOpacity: 0.82
    property string customOutDir: ""
    readonly property bool dragActive: dropArea.containsDrag
    readonly property string outLabel: customOutDir.length > 0 ? processor.displayName(customOutDir)
                                                               : qsTr("Auto (./out)")
    property alias processor: processor
    readonly property string formatList: processor.formats.join(", ")
    property alias results: results

    signal resetRequested()

    width: 880
    height: 620
    minimumWidth: 740
    minimumHeight: 520
    visible: true
    title: "img-settings"
    color: fluent ? Fluent.base : Theme.backdropBase
    // On macOS the glass runs under a transparent, unified title bar.
    flags: fluent ? Qt.Window : Qt.Window | Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint
    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    function start(urls) {
        if (processor.busy || urls.length === 0)
            return
        results.clear()
        wasCancelled = false
        const queued = processor.process(urls, recurse, watermarkOpacity, customOutDir)
        if (queued === 0)
            shell.item.notify(qsTr("No supported photos found (%1)").arg(formatList))
        if (queued > 0)
            showResults = true
    }

    function reset() {
        showResults = false
        results.clear()
        resetRequested()
    }

    function browseFolder() { folderDialog.open() }
    function browseFiles() { fileDialog.open() }
    function chooseOutput() { outputDialog.open() }

    Binding {
        target: Fluent
        property: "systemAccent"
        value: window.palette.accent
        when: window.fluent
    }

    Processor {
        id: processor
        onFileFinished: (name, output, outputUrl, bytes, error) => {
            // Files finish out of order on the thread pool; keep the grid sorted by name.
            let at = results.count
            while (at > 0 && results.get(at - 1).name.localeCompare(name, undefined, { numeric: true }) > 0)
                --at
            results.insert(at, { name: name, outputUrl: outputUrl.toString(), bytes: bytes, error: error,
                                  addedAt: Date.now() })
        }
        onFinished: (cancelled) => window.wasCancelled = cancelled
        onProblem: (message) => shell.item.notify(message)
    }

    ListModel { id: results }

    FolderDialog {
        id: folderDialog
        title: qsTr("Choose a folder of photos")
        onAccepted: window.start([selectedFolder])
    }

    FileDialog {
        id: fileDialog
        title: qsTr("Choose photos")
        fileMode: FileDialog.OpenFiles
        nameFilters: [qsTr("Photos (%1)").arg(processor.formats.map(
            f => f === "JPG" ? "*.jpg *.jpeg" : "*." + f.toLowerCase()).join(" "))]
        onAccepted: window.start(selectedFiles)
    }

    FolderDialog {
        id: outputDialog
        title: qsTr("Choose where to save watermarked photos")
        onAccepted: window.customOutDir = processor.pathFromUrl(selectedFolder)
    }

    Shortcut {
        sequences: [StandardKey.Open]
        enabled: !processor.busy
        onActivated: folderDialog.open()
    }
    Shortcut {
        sequence: "Esc"
        onActivated: processor.cancel()
    }

    Loader {
        id: shell
        anchors.fill: parent
        sourceComponent: window.fluent ? windowsShell : macShell
    }

    Component {
        id: macShell
        MacShell { app: window }
    }

    Component {
        id: windowsShell
        WindowsShell { app: window }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        enabled: !processor.busy
        onEntered: (drag) => {
            if (drag.hasUrls)
                drag.accept(Qt.CopyAction)
            else
                drag.accepted = false
        }
        onDropped: (drop) => {
            if (!drop.hasUrls)
                return
            drop.accept(Qt.CopyAction)
            window.start(drop.urls)
        }
    }
}
