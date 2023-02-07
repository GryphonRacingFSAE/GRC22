import QtQuick

Item {
    required property string title;
    property double value;
    property int precision;

    width: 500
    height: 18

    onValueChanged: () => {
        display_value.text = value.toFixed(1) // TODO: update individual precision values
    }

    Text {
        id: title_text
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
            left: title_text.right
        }
    }
}