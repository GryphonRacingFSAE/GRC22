import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Rectangle {
    id: root
    color: "black"

    readonly property int rows: 2
    readonly property int columns: 28
    readonly property int boxSize: 20
    //readonly property int temps: []
    
    
    GridLayout{
        anchors.centerIn: parent
        width: root.columns * boxSize
        height: root.rows * boxSize * 10 //random buffer
        rows: 5
        columns: 1
        rowSpacing: 10
        Repeater{
            model: 5
            Column{
                Text{
                    font.family: "Consolas"
                    font.pointSize: 10
                    color: "white"
                    text: "Segment #" + `${index+1}`
                }
                GridLayout {
                    id: tempGrid
                    width: root.columns * boxSize
                    height: root.rows * boxSize
                    columnSpacing: 0
                    rowSpacing: 0
                    rows: root.rows
                    columns: root.columns

                    Repeater{
                        model: 56
                        
                        Rectangle{
                            width: boxSize
                            height: boxSize
                            color: index % 2 == 1 ? "light green" : "green" 

                            Text{
                                anchors.centerIn: parent
                                font.family: "Consolas"
                                font.pointSize: 10
                                color: "black"
                                text: index
                            }
                        }
                    }
                }
            }
        }
    }
}