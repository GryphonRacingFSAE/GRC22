import QtQuick
import CAN.BMS
import CAN.EnergyMeter
import CAN.MotorController

Item {
    Rectangle {
        id: section_A

        width: parent.width/3
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
                title: "Current"
            }
            DataBox {
                title: "Over-current Fault"
            }
            DataBox {
                title: "Hardware Over-current Fault"
            }
            DataBox {
                title: "Current Sensor Fault"
            }

            SectionHeader {
                title: "Voltage"
            }
            DataBox {
                title: "Over-voltage Fault"
            }
            DataBox {
                title: "Under-voltage Fault"
            }
            DataBox {
                title: "Gate Driver Over-Voltage"
            }
            DataBox {
                title: "Hardware DC Bus Over-voltage Fault"
            }

            SectionHeader {
                title: "Temperature"
            }
            DataBox {
                title: "Inverter Over-temperature Fault"
            }
            DataBox {
                title: "Motor Over-temperature Fault"
            }
            DataBox {
                title: "PCB Over-temperature Fault"
            }
            DataBox {
                title: "Module A Over-temperature Fault"
            }
            DataBox {
                title: "Module B Over-temperature Fault"
            }
            DataBox {
                title: "Module C Over-temperature Fault"
            }
            DataBox {
                title: "Gate Drive Board 1 Over-temperature Fault"
            }
            DataBox {
                title: "Gate Drive Board 2 Over-temperature Fault"
            }
            DataBox {
                title: "Gate Drive Board 3 Over-temperature Fault"
            }
        }
    }

    Rectangle {
        id: section_B

        width: parent.width/3
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
                title: "Accelerator"
            }
            DataBox {
                title: "Accelerator Input Shorted Fault"
            }
            DataBox {
                title: "Accelerator Input Open Fault"
            }

            SectionHeader {
                title: "Brake"
            }
            DataBox {
                title: "Brake Input Shorted Fault"
            }
            DataBox {
                title: "Brake Input Open Fault"
            }

            SectionHeader {
                title: "Other"
            }
            DataBox {
                title: "Motor Over-speed Fault"
            }
            DataBox {
                title: "Direction Command Fault"
            }
            DataBox {
                title: "Inverter Response Time-out Fault"
            }
            DataBox {
                title: "Hardware Gate/Desaturation Fault"
            }
            DataBox {
                title: "CAN Command Message Lost Fault"
            }
            DataBox {
                title: "Resolver Not Connected"
            }
        }
    }

    Rectangle {
        id: section_C

        width: parent.width/3
        height: parent.height
        color: "black"
        anchors {
            top: parent.top
            left: section_B.right
        }

        Column {
            width: parent.width - 32
            height: parent.height
            anchors.horizontalCenter: parent.horizontalCenter
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