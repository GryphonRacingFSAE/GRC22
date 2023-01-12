import QtQuick

Item {
    required property string title;

    width: 500
    height: 60

    Text {
        text: parent.title
        font.pointSize: 12
        font.bold: true
        color: "white"

        anchors.verticalCenter: parent.verticalCenter
    }
}