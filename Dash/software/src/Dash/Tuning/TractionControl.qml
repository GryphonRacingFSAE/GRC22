import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Rectangle {
    id: root
    color: "black"
    readonly property int boxSize: 50
    property int selectedIndex: 0
    property bool selectedColumn: false

    Keys.onLeftPressed: () => {
        selectedColumn = !selectedColumn;
    }

    Keys.onRightPressed: () => {
        selectedColumn = !selectedColumn;
    }

    Keys.onUpPressed: () => {
        if (selectedIndex == 0){
            selectedIndex = 3;
        } else {
            selectedIndex--;
        }
    }

    Keys.onDownPressed: () => {
        if(selectedIndex == 3){
            selectedIndex = 0;
        } else{
            selectedIndex++;
        }
    }

    Keys.onDigit1Pressed: () => {
        if(selectedIndex == 3){
            tireCompound.val = !tireCompound.val;
        } else if(selectedColumn){
            //wheel slip & max reduction & driver weight
            if(selectedIndex == 0){
                wheelSlip.val += 1;
            } else if(selectedIndex == 1){
                maxReduction.val += 1;
            } else if(selectedIndex == 2){
                driverWeight.val += 1;
            } 
        } else {
            //pid
            pidValues.itemAt(selectedIndex).val += 1;
        }
    }

    Keys.onDigit2Pressed: () => {
        if(selectedIndex == 3){
            tireCompound.val = !tireCompound.val;
        } else if(selectedColumn){
            //wheel slip & max reduction & driver weight
            if(selectedIndex == 0){
                wheelSlip.val -= 1;
            } else if(selectedIndex == 1){
                maxReduction.val -= 1;
            } else if(selectedIndex == 2){
                driverWeight.val -= 1;
            } 
        } else {
            //pid
            pidValues.itemAt(selectedIndex).val += 1;
        }
    }

    Keys.onPressed: (event) => {
        switch (event.key) {
            case Qt.Key_S: {
                let tc_tune = [];
                for (let i = 0; i < 3; i++) {
                    tc_tune.push(pidValues.itemAt(i).val);
                }
                tc_tune.push(wheelSlip.val);
                tc_tune.push(maxReduction.val);

                VCU.saveTcTuneCSV(tc_tune); 
                break;
            }
            case Qt.Key_Plus: VCU.tcTuneId = Math.min(VCU.tcTuneId + 1, 3); break;
            case Qt.Key_Minus: VCU.tcTuneId = Math.max(VCU.tcTuneId - 1, 0); break;
        }
    }

    Keys.onReturnPressed: () => {
        let tc_tune = [];
        for (let i = 0; i < 3; i++) {
            tc_tune.push(pidValues.itemAt(i).val);
        }
        tc_tune.push(wheelSlip.val);
        tc_tune.push(maxReduction.val);
        VCU.sendTcTune(tc_tune);
    }

    Row{
        id: data
        spacing: 25
        anchors.centerIn: parent
        //PID
        Row{
            id: pidBox
            //PID Label
            Column {
                width: boxSize
                height: boxSize*3
                spacing: 0
                Repeater {
                    id: pid   
                    model: ["P", "I", "D"]
                    Rectangle{
                        width: boxSize
                        height: boxSize
                        color: "black"

                        required property string modelData
                        Text{
                            anchors.centerIn: parent
                            font.family: "Consolas"
                            font.pointSize: 10
                            color: "white"
                            text: modelData
                        }
                    }
                }
            }
            //PID Values
            Column {
                width: boxSize
                height: boxSize*3
                spacing: 0
                

                Repeater {
                    id: pidValues   
                    model: [VCU.currentTcTune[0], VCU.currentTcTune[1], VCU.currentTcTune[2]]
                    Rectangle{
                        width: boxSize
                        height: boxSize
                        color: index == 1 ? "light green" : "green" 

                        property bool isSelected: index == root.selectedIndex && selectedColumn == 0
                        property real val: modelData.toFixed(2)
                        border{
                            color: "blue"
                            width: isSelected ? 3 : 0
                        }
                        Text{
                            anchors.centerIn: parent
                            font.family: "Consolas"
                            font.pointSize: 10
                            color: "black"
                            text: pidValues.itemAt(index).val
                        }
                    }
                }
            }
        }

        //Wheel slip angle and max reduction
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
                        text: "Wheel Slip Angle"
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
                        text: "Max Reduction"
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
                        text: "Driver Weight"
                    }
                }
            }

            Column{
                width: boxSize
                Rectangle{
                    id: wheelSlip
                    width: boxSize
                    height: boxSize
                    color: "green"
                    property bool isSelected: root.selectedIndex == 0 && selectedColumn == 1
                    property int val: VCU.currentTcTune[3]
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
                    id: maxReduction
                    width: boxSize
                    height: boxSize
                    color: "light green"
                    property bool isSelected: root.selectedIndex == 1 && selectedColumn == 1
                    property int val: VCU.currentTcTune[4]
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
                    id: driverWeight
                    width: boxSize
                    height: boxSize
                    color: "green"
                    property bool isSelected: root.selectedIndex == 2 && selectedColumn == 1
                    property int val: VCU.currentTcTune[5]
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

    Column{
        spacing: 20
        anchors{
            top: data.bottom
            horizontalCenter: data.horizontalCenter
            topMargin: 20
        }
        Row{
            spacing: 20
            Text{             
                anchors.verticalCenter: parent.verticalCenter
                font.family: "Consolas"
                font.pointSize: 10
                color: "white"
                text: "Tire Compound"
            }

            Rectangle{
                id: tireCompound
                width: boxSize
                height: boxSize
                color: "green"
                property bool isSelected: root.selectedIndex == 3
                property int val: VCU.currentTcTune[6]
                border{
                    color: "blue"
                    width: isSelected ? 3 : 0
                }
                Text{                        
                    anchors.centerIn: parent
                    font.family: "Consolas"
                    font.pointSize: 10
                    color: "black"
                    text: tireCompound.val ? "LC0" : "R20" 
                }
            }
        }

        //Profile Num
        Text{
            font.family: "Consolas"
            font.pixelSize: 24
            font.bold: true
            color: "white"
            text: "Profile #" + `${VCU.tcTuneId}`
        }
    }

    //Title: Traction Control
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
        text: "Traction Control"
    }
    
}