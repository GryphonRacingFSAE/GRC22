import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU


Rectangle {
    id: root
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

        Repeater{
            id: repeater
            model: segments

            Column{
                property int num: modelData
                spacing: 0
                Text{
                    font.family: "Consolas"
                    font.pointSize: 10
                    color: "white"
                    text: "Segment #" + `${num}`
                }

                Segment{
                    type: 0 //type 0 = voltages from bms
                    segment: num
                    max: 5
                    min: 1
                    boxSize: 35
                    rows: 2
                    columns: 14
                }
            }
        }
    }
}


    
    
