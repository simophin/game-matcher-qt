import QtQuick 2.4
import nz.cloudwalker 1.0

NewClubPageForm {
    signal clubSaved(string path)
    signal clubSaveError(string msg)
    signal cancelled()

    property string filePath

    ClubRepository {
        id: repo
    }

    saveBtn.onClicked: {
        repo.dbPath = filePath
        if (!repo.isValid || repo.dbPath != filePath) {
            clubSaveError(qsTr("Unable to save club file"))
        } else {
            repo.saveClubSettings(clubName.text, parseFloat(sessionFee.text) * 100);
            repo.close()
            clubSaved(filePath)
        }
    }

    cancelBtn.onClicked: cancelled()
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
