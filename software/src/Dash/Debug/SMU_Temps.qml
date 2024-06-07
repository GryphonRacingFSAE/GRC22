import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU

Rectangle {
    id: root
    color: "black"
    property int boxSize: 27
    property int segments: 5

    Column{
        anchors.centerIn: parent
        spacing: 5
        Rectangle{
            width: root.width
            height: boxSize*5
            color: "transparent"
            Text{
                anchors{
                    verticalCenter: parent.verticalCenter
                    left: parent.left
                    leftMargin: 10
                }

                font.family: "Consolas"
                font.pixelSize: 30
                font.bold: true
                color: "white"
                text: "Thermistor Temperatures (°C)"
            }

            Rectangle{
                anchors.right: parent.right
                anchors.rightMargin: 10
                color: "transparent"
                height: parent.height + 6
                width: 6*boxSize 
                border.width: 2
                border.color: "White"

                Text{
                    anchors{
                        verticalCenter: parent.verticalCenter
                        left: parent.left
                        leftMargin: 5
                    }
                    font.family: "Consolas"
                    font.pixelSize: 15
                    color: "white"
                    text: "Segments \nMin/Max"
                }

                Column{
                    spacing: 0
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 2.5
                    Repeater{
                        model: segments
                        Row{
                            spacing:0
                            property int seg: modelData

                            Text{
                                font.family: "Consolas"
                                width: boxSize
                                height: boxSize
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                                text: `${seg}` + "{"
                                color: "white"
                            }

                            //minimum box
                            Rectangle{
                                width: boxSize
                                height: boxSize
                                property int seg: modelData

                                color: Qt.rgba(0, 0, 1, 1)

                                Text{
                                    font.family: "Consolas"
                                    width: parent.width
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    text: {
                                        //gets min
                                        var temp = 40;
                                        for(var i = 0; i<28; i++){
                                            if(!SMU.filtered[seg*28 + i] && SMU.temperatures[seg*28 + i] < temp){
                                                temp = SMU.temperatures[seg*28 + i];
                                            }
                                        }
                                        return temp.toFixed();
                                    } 
                                    font.pointSize: 8
                                }
                            }
                            //maximum box
                            Rectangle{
                                width: boxSize
                                height: boxSize

                                color: Qt.rgba(1, 0, 0, 1)

                                Text{
                                    font.family: "Consolas"
                                    width: parent.width
                                    height: parent.height
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                    text: {
                                        //gets min
                                        var temp = -40;
                                        for(var i = 0; i<28; i++){
                                            if(!SMU.filtered[seg*28 + i] && SMU.temperatures[seg*28 + i] > temp){
                                                temp = SMU.temperatures[seg*28 + i];
                                            }
                                        }
                                        return temp.toFixed();
                                    } 
                                    font.pointSize: 8
                                }
                            }
                        }
                    }
                }
            }
        }

        Text{
            anchors{
                left: parent.left
                leftMargin: 10
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
                anchors.horizontalCenter: parent.horizontalCenter
                property int num: modelData
                spacing: 0
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
                    max: 70
                    min: 10
                    boxSize: root.boxSize
                    rows: 2
                    columns: 28
                }
            }
        }
    }
}