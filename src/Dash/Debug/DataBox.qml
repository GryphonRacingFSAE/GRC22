import QtQuick

Item {
    required property string title;
    property double value;
    property int precision;

    width: parent.width
    height: 18

    onValueChanged: () => {
        display_value.text = value.toFixed(1) // TODO: update individual precision values
    }

    Text {
        text: parent.title + ":  "
        font.pointSize: 10
        color: "white"
    }

    Text {
        id: display_value
        text: ""
        font.pointSize: 10
        font.bold: true
        color: "green"
        
        anchors {
            right: parent.right
        }
    }
}