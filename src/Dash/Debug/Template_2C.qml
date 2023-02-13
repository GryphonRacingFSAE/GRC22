import QtQuick
import CAN.BMS
import CAN.EnergyMeter
import CAN.MotorController

Item {
    Rectangle {
        id: section_A

        width: parent.width/2
        height: parent.height
        color: "black"
        anchors {
            top: parent.top
            left: parent.left
        }

        Column {
            width: parent.width - 32
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter

            SectionHeader {
                title: "HEADER"
            }
            DataBox {
                title: "DATA BOX"
            }
        }
    }

    Rectangle {
        id: section_B

        width: parent.width/2
        height: parent.height
        color: "black"
        anchors {
            top: parent.top
            left: section_A.right
        }

        Column {
            width: parent.width - 32
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter

            SectionHeader {
                title: "HEADER"
            }
            DataBox {
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