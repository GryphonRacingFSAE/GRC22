import QtQuick
import CAN.BMS
import CAN.EnergyMeter
import CAN.MotorController

Rectangle {
    color: "black"

    Item {
        id: section_A

        width: parent.width/2
        height: parent.height
        anchors {
            top: parent.top
            left: parent.left
        }

        Column {
            leftPadding: 24

            SectionHeader {
                title: "HEADER"
            }
            DataBox {
                id: data_box_A
                title: "DATA BOX"
            }
        }
    }

    Item {
        id: section_B

        width: parent.width/2
        height: parent.height
        anchors {
            top: parent.top
            left: section_A.right
        }

        Column {
            leftPadding: 12

            SectionHeader {
                title: "HEADER"
            }
            DataBox {
                id: data_box_B
                title: "DATA BOX"
            }
        }
    }

    Connections {
        target: BMS

    }

    Connections {
        target: EnergyMeter

    }

    Connections {
        target: MotorController

    }
}