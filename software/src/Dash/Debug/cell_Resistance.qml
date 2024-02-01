import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.BMS


Rectangle {
    id: root
    property int boxSize: 36
    color: "black"
    
    readonly property int segments: 5
    //column that holds everything
    Column{
        anchors.centerIn: parent
        spacing: 5
        //title
        Text{
            anchors.horizontalCenter: parent.horizontalCenter

            font.family: "Consolas"
            font.pixelSize: 24
            font.bold: true
            color: "white"
            text: "Resistances (mOhm)"
        }
        //Segment and min/max label
        Rectangle{
            color: "transparent"
            width: parent.width
            height: 15
            Text{
                anchors{
                    left: parent.left
                }

                font.family: "Consolas"
                font.pixelSize: 15
                color: "white"
                text: "Segments"
            }

            Text{
                anchors{
                    right: parent.right
                }

                font.family: "Consolas"
                font.pixelSize: 15
                color: "white"
                text: "Min/Max "
            }
        }
        //repeater that shows 5 segments, labels and their min/max
        Repeater{
            id: repeater
            model: segments

            Row{
                property int seg: modelData
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
                        text: `${seg}` + " {"
                    }
                }

                Segment{
                    id: grid
                    type: 1 //type 1 = resistances from bms
                    segment: seg
                    min: 3.5
                    max: 7
                    boxSize: root.boxSize
                    rows: 2
                    columns: 14
                }

                //min/max boxes
                Rectangle{
                    width: boxSize*2
                    height: boxSize*2
                    color: "transparent"

                    Row{
                        anchors.centerIn: parent
                        spacing: 0
                        //minimum box
                        Rectangle{
                            width: boxSize
                            height: boxSize
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
                                        if(BMS.resistances[seg*28 + i] < temp){
                                            temp = BMS.resistances[seg*28 + i];
                                        }
                                    }
                                    return temp.toFixed(1);
                                } 
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
                                    //gets max
                                    var temp = -40;
                                    for(var i = 0; i<28; i++){
                                        if(BMS.resistances[seg*28 + i] > temp){
                                            temp = BMS.resistances[seg*28 + i];
                                        }
                                    }
                                    return temp.toFixed(1);
                                } 
                            }
                        }   
                    }
                }
            }
        }
    }
}