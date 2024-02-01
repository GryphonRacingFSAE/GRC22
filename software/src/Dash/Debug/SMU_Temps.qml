import QtQuick
import QtQuick.Controls
import QtQuick.Layouts


Rectangle {
    id: root
    color: "black"
    property int boxSize: 27
    property int segments: 5

    Column{
        anchors.centerIn: parent
        spacing: 5
        Text{
            anchors.horizontalCenter: parent.horizontalCenter

            font.family: "Consolas"
            font.pixelSize: 24
            font.bold: true
            color: "white"
            text: "Thermistor Tempuratures"
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
                    width: boxSize
                    height: boxSize*2
                    color: "transparent"
                    Text{
                        anchors.centerIn: parent
                        font.family: "Consolas"
                        font.pointSize: 20
                        color: "white"
                        text: `${num}` + "{"
                    }
                }
                Segment{
                    id: segDisplay
                    type: 2 //type 2 = temperatures from smu
                    segment: num
                    max: 100
                    min: 1
                    boxSize: root.boxSize
                    rows: 2
                    columns: 28
                }
            }
        }
    }
}