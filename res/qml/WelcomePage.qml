import QtQuick 2.4
import QtQuick.Dialogs 1.2

WelcomePageForm {
    signal clubFileReady(string path)

    openClubBtn.onClicked: {
        openFileDialog.visible = true
    }

    NewClubPage {
        id: newClub
        anchors.fill: parent
        visible: false
        onCancelled: stack.pop()
        onClubSaved: {
            clubFileReady(filePath)
            stack.pop()
        }
    }

    newClubBtn.onClicked: {
        newFileDialog.visible = true
    }

    FileDialog {
        id: newFileDialog
        title: qsTr("New club file")
        selectExisting: false
        onAccepted: stack.push(newClub)
    }

    FileDialog {
        id: openFileDialog
        title: qsTr("Choose a club file")
        onAccepted: {
            clubFileReady(fileUrl)
        }
        visible: false
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
