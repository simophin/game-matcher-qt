import QtQuick 2.4
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

StackView {
    id: stack
    anchors.fill: parent
    property alias openClubBtn: openClubBtn
    property alias newClubBtn: newClubBtn
    property alias stack: stack

    initialItem: initial

    ColumnLayout {
        id: initial
        spacing: 0
        anchors.fill: parent

        Item {
            Layout.fillHeight: true
        }

        Text {
            id: element
            text: qsTr("Welcome")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            font.pixelSize: 30
        }

        Button {
            id: openClubBtn
            text: qsTr("Open a club file")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: false
            highlighted: true
        }

        Button {
            id: newClubBtn
            text: qsTr("Create a new club file")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        Item {
            Layout.fillHeight: true
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;formeditorColor:"#ffffff";height:480;width:640}
}
##^##*/

