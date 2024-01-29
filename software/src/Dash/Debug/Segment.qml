import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU
import CAN.BMS


Rectangle {  
    id: root 
    required property int type
    required property int segment
    required property int max
    required property int min
    required property int boxSize
    required property int rows
    required property int columns
    

    color: "white"
    width: columns * boxSize
    height: rows * boxSize

    

    GridLayout{
        width: root.columns * boxSize
        height: root.rows * boxSize
        anchors.centerIn: parent

        rows: root.rows
        columns: root.columns

        columnSpacing: 0
        rowSpacing: 0

        Repeater {
            id: repeater
            model: rows*columns

            Rectangle{
                width: boxSize
                height: boxSize

                property double value: {
                    if(root.type == 0){
                        return BMS.voltages[root.segment*(root.rows*root.columns) + index]
                    } else if(root.type == 1){
                        return BMS.resistances[root.segment*(root.rows*root.columns) + index]
                    } else if(root.type == 2){
                        return SMU.temperatures[root.segment*(root.rows*root.columns) + index]
                    } 

                }

                Text{
                    font.family: "Consolas"
                    width: parent.width
                    height: parent.height
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    text: {
                        if(root.type == 0 || root.type == 1){
                            return value.toFixed(1)
                        } 
                        return value
                    }
                    font.pointSize: 10
                }


                color: {
                    if (repeater.itemAt(index).value < 0) {
                        return Qt.rgba(0, 0, 1, -(value/root.max))
                    } else {
                        return Qt.rgba(1, 0, 0, value/root.max)
                    }
                }
            }
        }
    }
}