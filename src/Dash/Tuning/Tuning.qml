import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Rectangle {
    id: root
    color: "black"

    readonly property int rows: 11
    readonly property int columns: 14
    readonly property int boxSize: 40

    property int selectedRow: 0
    property int selectedColumn: 0
    readonly property int selectedIndex: selectedRow*columns + selectedColumn // index counts down columns for whatever reason
    Keys.onLeftPressed: () => {
        if (selectedColumn == 0){
            selectedColumn = columns - 1;
        } else {
            selectedColumn--;
        }
    }
    Keys.onRightPressed: () => {
        if (selectedColumn == columns - 1){
            selectedColumn = 0;
        } else {
            selectedColumn++;
        }
    }
    Keys.onUpPressed: () => {
        if (selectedRow == 0){
            selectedRow = rows - 1;
        } else {
            selectedRow--;
        }
    }
    Keys.onDownPressed: () => {
        if (selectedRow == rows - 1){
            selectedRow = 0;
        } else {
            selectedRow++;
        }
    }
    Keys.onDigit1Pressed: () => {
        repeater.itemAt(selectedIndex).torque = Math.max(repeater.itemAt(selectedIndex).torque - 1, VCU.minTorque)
    }
    Keys.onDigit2Pressed: () => {
        repeater.itemAt(selectedIndex).torque = Math.min(repeater.itemAt(selectedIndex).torque + 1, VCU.maxTorque)
    }
    Keys.onPressed: (event) => {
        switch (event.key) {
            case Qt.Key_S: {
                // Array must be in row-major order to work as expected (each row is pedal position)
                let torque_map = [];
                for (let i = 0; i < rows * columns; i++) {
                    torque_map.push(repeater.itemAt(i).torque);
                }
                VCU.saveTorqueMapCSV(torque_map); 
                break;
            }
            case Qt.Key_Plus: VCU.profileId = Math.min(VCU.profileId + 1, 3); break;
            case Qt.Key_Minus: VCU.profileId = Math.max(VCU.profileId - 1, 0); break;
        }
    }
    Keys.onReturnPressed: () => {
        // Array must be in row-major order to work as expected (each row is pedal position)
        let torque_map = [];
        for (let i = 0; i < rows * columns; i++) {
            torque_map.push(repeater.itemAt(i).torque);
        }
        VCU.sendTorqueMap(torque_map);
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        Label {
            text: repeater.itemAt(selectedIndex)?.torque ?? "Null"
            color: "white"
            font.pixelSize: 18
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: `Profile #${VCU.profileId}`
            color: "white"
            font.pixelSize: 18
        }
    }
    Rectangle {
        color: "white"
        anchors.centerIn: parent
        width: root.columns * boxSize
        height: root.rows * boxSize
        GridLayout {
            anchors.centerIn: parent
            width: root.columns * boxSize
            height: root.rows * boxSize

            rows: root.rows
            columns: root.columns

            columnSpacing: 0
            rowSpacing: 0
            Repeater {
                id: repeater
                model: VCU.currentTorqueMap
                Rectangle {
                    property int torque: modelData

                    property bool isSelected: index == root.selectedIndex
                    border {
                        color: "blue"
                        width: isSelected ? 3 : 0
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: {
                        if (torque < 0) {
                            return Qt.rgba(1, 0, 0, torque/VCU.minTorque)
                        } else {
                            return Qt.rgba(0, 1, 0, torque/VCU.maxTorque)
                        }
                    }
                }
            }
        }
    }
}