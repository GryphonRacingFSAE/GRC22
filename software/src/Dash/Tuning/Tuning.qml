import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Rectangle {
    id: root
    color: "black"
    readonly property int boxSize: 50
    property int selectedIndex: 0

    Keys.onUpPressed: () => {
        selectedIndex = Math.max(0, selectedIndex - 1);
    }

    Keys.onDownPressed: () => {
        selectedIndex = Math.min(selectedIndex + 1, 3);
    }

    Keys.onLeftPressed: () => {
        //wheel slip & max reduction & driver weight
        if(selectedIndex == 0){
            maxTorque.val = Math.max(maxTorque.val - 5, 10);
        } else if(selectedIndex == 1){
            maxPower.val = Math.max(maxPower.val - 5, 5);
        } else if(selectedIndex == 2){
            maxRPM.val = Math.max(maxRPM.val - 250, 1500);
        } else if(selectedIndex == 3){
            pumpIdleSpeed.val = Math.max(pumpIdleSpeed.val - 5, 5);
        } 
    }

    Keys.onRightPressed: () => {
        //wheel slip & max reduction & driver weight
        if(selectedIndex == 0){
            maxTorque.val = Math.min(maxTorque.val + 5, 230);
        } else if(selectedIndex == 1){
            maxPower.val = Math.min(maxPower.val + 5, 80);
        } else if(selectedIndex == 2){
            maxRPM.val = Math.min(maxRPM.val + 250, 6500);
        } else if(selectedIndex == 3){
            pumpIdleSpeed.val = Math.min(pumpIdleSpeed.val + 5, 100);
        }
    }

    Keys.onReturnPressed: () => {
        VCU.sendTorqueConfig(maxTorque.val, maxPower.val, maxRPM.val, pumpIdleSpeed.val);
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
                
                Rectangle{
                    width: boxSize*4
                    height: boxSize
                    color: "black"
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "white"
                        text: "Max RPM"
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
                        text: "Pump Idle Speed"
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
                    property int val: 120
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
                    property int val: 70
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
                    id: maxRPM
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property bool isSelected: root.selectedIndex == 2
                    property int val: 6000
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
                    id: pumpIdleSpeed
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property bool isSelected: root.selectedIndex == 3
                    property int val: 20
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
            
            Column{
                width: boxSize
                Rectangle{
                    id: storedMaxTorque
                    width: boxSize
                    height: boxSize
                    color: "green"
                    property int val: -1
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "black"
                        text: parent.val
                    }
                }
                Rectangle{
                    id: storedMaxPower
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property int val: -1
                    Text{                        
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "black"
                        text: parent.val
                    }
                }
                Rectangle{
                    id: storedMaxRPM
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property int val: -1
                    Text{                        
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 10
                        color: "black"
                        text: parent.val
                    }
                }
                Rectangle{
                    id: storedPumpIdleSpeed
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property int val: -1
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
    Connections {
        target: VCU
        function onNewStoredTorque(value) {
            storedMaxTorque.val = value;
        }
        function onNewStoredPower(percent) {
            storedMaxPower.val = percent;
        }
        function onNewStoredSpeedLimit(state) {
            storedMaxRPM.val = state;
        }
        function onNewStoredIdlePumpSpeed(state) {
            storedPumpIdleSpeed.val = state;
        }
    }
}