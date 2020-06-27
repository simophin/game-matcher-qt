import QtQuick 2.4
import nz.cloudwalker 1.0

ClubPageForm {
    ClubRepository {
        id: repo
    }

    clubHeader.text: repo.clubInfo.name

}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
