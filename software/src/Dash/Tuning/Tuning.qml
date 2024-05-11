import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Rectangle {
    id: root
    color: "black"
    readonly property int boxSize: 50
    property int selectedIndex: 0

    Keys.onLeftPressed: () => {
        selectedIndex = !selectedIndex;
    }

    Keys.onRightPressed: () => {
        selectedIndex = !selectedIndex;
    }

    Keys.onUpPressed: () => {
        //wheel slip & max reduction & driver weight
        if(selectedIndex == 0){
            maxTorque.val += 50;
        } else if(selectedIndex == 1){
            maxPower.val += 50;
        } 
    }

    Keys.onDownPressed: () => {
        //wheel slip & max reduction & driver weight
        if(selectedIndex == 0){
            maxTorque.val -= 50;
        } else if(selectedIndex == 1){
            maxPower.val -= 50;
        }
    }

    Keys.onReturnPressed: () => {
        VCU.sendTorqueConfig(maxTorque, maxPower);
    }
    Row{
        id: data
        spacing: 25
        anchors.centerIn: parent

        Row{
            spacing: 1
            Column{
                Rectangle{
                    width: boxSize*4
                    height: boxSize
                    color: "black"
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "white"
                        text: "Max Torque"
                    }
                }

                Rectangle{
                    width: boxSize*4
                    height: boxSize
                    color: "black"
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "white"
                        text: "Max Power"
                    }
                }
            }

            Column{
                width: boxSize
                Rectangle{
                    id: maxTorque
                    width: boxSize
                    height: boxSize
                    color: "green"
                    property bool isSelected: root.selectedIndex == 0
                    property int val: VCU.maxTorque
                    border{
                        color: "blue"
                        width: isSelected ? 3 : 0
                    }
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "black"
                        text: parent.val
                    }
                }
                Rectangle{
                    id: maxPower
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property bool isSelected: root.selectedIndex == 1
                    property int val: VCU.maxPower
                    border{
                        color: "blue"
                        width: isSelected ? 3 : 0
                    }
                    Text{                        
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "black"
                        text: parent.val
                    }
                }
            }
        }
    }

    Text{
        anchors{
            horizontalCenter: data.horizontalCenter
            bottom: data.top
            margins: 30
        }
        font.family: "Consolas"
        font.pixelSize: 30
        font.bold: true
        color: "white"
        text: "Car Paramters"
    }
}