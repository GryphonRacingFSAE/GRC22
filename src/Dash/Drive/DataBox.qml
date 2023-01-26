import QtQuick

Item {
    required property string title;
    required property double fontSize;
    required property double low;
    required property double high;
    required property int precision;
    property double value;

    onValueChanged: () => {
        display_value.text = value.toFixed(precision)
        if (value < low) {
            display_value.color="blue"
        } else if(value > high) {
            display_value.color="red"
        } else {
            display_value.color="green"
        }
    }

    Text {
        font.pointSize: parent.fontSize
        text: parent.title
        color: "black"
        font.bold: true
        anchors{
            bottom: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }
    }

    Text {
        id: display_value
        font.pointSize: parent.fontSize * 20 / 8
        text: ""
        color: "black"
        font.bold: true
        anchors{
            top: parent.verticalCenter
            horizontalCenter: parent.horizontalCenter
        }
    }
}