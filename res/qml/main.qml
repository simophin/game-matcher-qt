import QtQuick 2.12
import QtQuick.Controls 2.5
import Qt.labs.settings 1.0

ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480

    property string currentClubFile

    StackView {
        id: stackView
        anchors.fill: parent
    }

    onCurrentClubFileChanged: {
        var page
        if (!currentClubFile) {
            page = Qt.createComponent("WelcomePage.qml").createObject(window)
            page.clubFileReady.connect(onClubFileReady)
        } else {
            page = Qt.createComponent("HomeForm.ui.qml").createObject(window)
            console.log("Replacing with home")
        }

        stackView.replace(page)
    }

    function onClubFileReady(path) {
        currentClubFile = path
    }

    function onClubClosed() {
        delete currentClubFile
    }

    Settings {
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height
        property alias currentClubFile: window.currentClubFile
    }
}
