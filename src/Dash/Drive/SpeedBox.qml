import QtQuick

Item {
  required property string title;
  required property double fontSize;
  required property int precision;
  property double value;

  onValueChanged: () => {
  display_value.text = value.toFixed(precision)}

  Text {
    font.pointSize: parent.fontSize
    text: parent.title
    color: "black"
    font.bold: true
    anchors {
      bottom: parent.verticalCenter
      horizontalCenter: parent.horizontalCenter
    }
  }

  Text {
    id: display_value
    font.pointSize: parent.fontSize * 20 / 6
    text: ""
    color: "black"
    font.bold: true
    anchors {
      top: parent.verticalCenter
      horizontalCenter: parent.horizontalCenter
    }
  }
}