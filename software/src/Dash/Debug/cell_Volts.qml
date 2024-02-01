import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU


Rectangle {
    id: root
    property int boxSize: 36
    color: "black"
    
    readonly property int segments: 5

    Column{
        anchors.centerIn: parent
        spacing: 5
        Text{
            anchors.horizontalCenter: parent.horizontalCenter

            font.family: "Consolas"
            font.pixelSize: 24
            font.bold: true
            color: "white"
            text: "Voltages"
        }

        Text{
            anchors{
                left: parent.left
            }

            font.family: "Consolas"
            font.pixelSize: 15
            color: "white"
            text: "Segments"
        }

        Repeater{
            id: repeater
            model: segments

            Row{
                property int num: modelData
                spacing: 3
                Rectangle{
                    width: boxSize*2
                    height: boxSize*2
                    color: "transparent"
                    Text{
                        anchors{
                            verticalCenter: parent.verticalCenter
                            right: parent.right
                        }

                        font.family: "Consolas"
                        font.pointSize: 20
                        color: "white"
                        text: `${num}` + " {"
                    }
                }

                Segment{
                    id: grid
                    type: 0 //type 0 = voltages from bms
                    segment: num
                    max: 5
                    min: 1
                    boxSize: root.boxSize
                    rows: 2
                    columns: 14
                }
            }
        }
    }
}


    
    
