import QtQuick 2.9
import QtQuick.Layouts 1.11
import QtQuick.Controls 2.15

ColumnLayout {
    property alias clubName: clubName
    property alias sessionFee: sessionFee
    property alias saveBtn: saveBtn
    property alias cancelBtn: cancelBtn

    anchors.fill: parent

    Text {
        text: qsTr("Club infomation")
        font.bold: true
        Layout.margins: 8
        font.pixelSize: 24
    }

    TextField {
        id: clubName
        Layout.margins: 8
        Layout.rowSpan: 0
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        placeholderText: qsTr("Club name")
    }

    TextField {
        id: sessionFee
        Layout.margins: 8
        Layout.rowSpan: 0
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignLeft | Qt.AlignTop
        placeholderText: qsTr("Fee per session")
        validator: DoubleValidator {
            decimals: 2
            bottom: 0.0
        }
    }

    Item {
        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    RowLayout {
        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.fillWidth: false
        Button {
            id: saveBtn
            text: qsTr("Save")
            highlighted: true
            enabled: clubName.text.length > 0 && sessionFee.acceptableInput
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        Button {
            id: cancelBtn
            text: qsTr("Cancel")
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

