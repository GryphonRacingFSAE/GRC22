import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU
import CAN.BMS


Rectangle {  
    id: root 
    required property int type
    required property int segment
    required property double max
    required property double min
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
                            return value.toFixed(2)
                        } 
                        return value.toFixed()
                    }
                    font.pointSize: {
                        return 8
                    }
                }


                color: {
                    if(value > root.max){
                        //red above max
                        return Qt.rgba(1,0,0,1)
                    }else if(value < root.min){
                        //shade of blue below min
                        return Qt.rgba(0,0,1, 0.1 + 0.9*Math.abs((value-root.min)/(root.max-root.min)))
                    }else if(value >= root.min && value <= root.max){
                        //shade of green within range
                        return Qt.rgba(0,1,0, 0.3 + 0.7*Math.abs((value-root.min)/(root.max-root.min)))
                    }
                }
            }
        }
    }
}