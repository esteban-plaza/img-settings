import QtQuick

// macOS 26 look: Liquid Glass panels floating over a living backdrop.
Item {
    id: shell
    required property var app

    function notify(message) { toast.show(message) }

    Connections {
        target: shell.app.processor
        function onFileFinished(name, output, outputUrl, bytes, error) {
            if (error.length === 0)
                backdrop.photo = outputUrl
        }
    }
    Connections {
        target: shell.app
        function onResetRequested() { backdrop.photo = "" }
    }

    Backdrop {
        id: backdrop
        anchors.fill: parent
    }

    // The title bar is transparent and our content sits under it, so the strip
    // above the glass has to move the window itself (and zoom on double-click).
    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: mainPanel.y

        DragHandler {
            target: null
            onActiveChanged: if (active) shell.app.startSystemMove()
        }
        TapHandler {
            onDoubleTapped: shell.app.visibility === Window.Maximized ? shell.app.showNormal()
                                                                       : shell.app.showMaximized()
        }
    }

    GlassPanel {
        id: mainPanel
        backdrop: backdrop
        x: 20
        y: 44
        width: parent.width - 40
        height: toolbar.y - y - 16
        radius: 30
        scale: shell.app.dragActive ? 1.012 : 1
        tintColor: shell.app.dragActive ? Theme.accent : Theme.glassTint
        tintAmount: shell.app.dragActive ? 0.2 : Theme.glassTintAmount
        Behavior on scale { NumberAnimation { duration: Theme.spring; easing.type: Easing.OutBack; easing.overshoot: 2 } }
        Behavior on tintAmount { NumberAnimation { duration: 200 } }

        DropContent {
            anchors.fill: parent
            opacity: shell.app.showResults ? 0 : 1
            visible: opacity > 0
            dragActive: shell.app.dragActive
            formats: shell.app.processor.formats
            onBrowseFolder: shell.app.browseFolder()
            onBrowseFiles: shell.app.browseFiles()
            Behavior on opacity { NumberAnimation { duration: 250 } }
        }

        ResultsContent {
            anchors.fill: parent
            opacity: shell.app.showResults ? 1 : 0
            visible: opacity > 0
            processor: shell.app.processor
            results: shell.app.results
            cancelled: shell.app.wasCancelled
            onProcessMore: shell.app.reset()
            Behavior on opacity { NumberAnimation { duration: 250 } }
        }
    }

    Toolbar {
        id: toolbar
        backdrop: backdrop
        anchors.horizontalCenter: parent.horizontalCenter
        y: parent.height - height - 18
        enabled: !shell.app.processor.busy
        recurse: shell.app.recurse
        watermarkOpacity: shell.app.watermarkOpacity
        customOutput: shell.app.customOutDir.length > 0
        outLabel: shell.app.outLabel
        onRecurseToggled: shell.app.recurse = !shell.app.recurse
        onOpacityMoved: (value) => shell.app.watermarkOpacity = value
        onChooseOutput: shell.app.chooseOutput()
        onClearOutput: shell.app.customOutDir = ""
    }

    Toast {
        id: toast
        backdrop: backdrop
        anchors.horizontalCenter: parent.horizontalCenter
        y: 56
    }
}
