import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.VCU

Item{
    id: root
    required property int borderWidth;
    required property string lineColour;
    required property string letter;
    property int tickWidth: 10
    property int tickHeight: 2
    property double value;

    Rectangle{
        width: root.width
        height: root.height
        property double value

        color: "white"
        border{
            color: "black"
            width: borderWidth
        }

        //position bar
        Rectangle{
            height: (root.height - 2*borderWidth) * (root.value/100) //100% = root.height- 2*borderWidth
            width: root.width - borderWidth*2


            gradient: Gradient{
                GradientStop { position: 0; color: root.lineColour}
                GradientStop{ position: 1; color: "black"}
            }

            anchors{
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: root.borderWidth
            }
        }
        //label
        Text{
            text: root.letter
            font.family: "Consolas"
            font.pointSize: 20
            font.bold: true
            color: "white"

            anchors{
                horizontalCenter: parent.horizontalCenter
                bottom: parent.bottom
                bottomMargin: root.borderWidth
            }
        }

        //tick markers
        Repeater{
            model: 10
            Rectangle{
                required property int modelData
                width: root.tickWidth
                height: root.tickHeight
                color: "black"
                anchors{
                    right: parent.right
                    bottom: parent.bottom
                    bottomMargin: (root.height - 2*borderWidth) * (0.1*modelData)
                }
            }
        }
    }
}