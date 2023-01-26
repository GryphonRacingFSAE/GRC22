import QtQuick

Item {
    required property string title;
    property double value;
    property int precision;
    property double low;
    property double high;

    width: 500
    height: 20

    onValueChanged: () => {
        display_value.text = value.toFixed(precision)
        if (value < low) {
            display_value.color="blue"
        } else if (value > high) {
            display_value.color="red"
        } else {
            display_value.color="green"
        }
    }

    Text {
        text: parent.title + ": "
        font.pointSize: 10
        color: "white"
    }

    Text {
        id: display_value
        text: ""
        font.pointSize: 10
        color: "white"
    }
}