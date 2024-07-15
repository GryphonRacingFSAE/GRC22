import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Item{
    id: root
    required property string name;
    property string color: "green";
    property bool value: false;

    width: 50
    height: 50
    

    Rectangle{
        id: indicator
        width: 25
        height: 25
        property double value
        anchors.centerIn: root

        color: root.value ? root.color : Qt.tint(root.color, "#C0000000");
        border {
            color: "black"
            width: 3
        }
    }

    Text {
        text: name;
        width: 50;
        anchors.top: indicator.bottom
        horizontalAlignment: Text.AlignHCenter
    }
}