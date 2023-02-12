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
                title: "Current Sensor Low"
            }
            DataBox {
                title: "Current Sensor High"
            }

            SectionHeader {
                title: "Voltage"
            }
            DataBox {
                title: "1.5V Sense Voltage Low"
            }
            DataBox {
                title: "1.5V Sense Voltage High"
            }
            DataBox {
                title: "2.5V Sense Voltage Low"
            }
            DataBox {
                title: "2.5V Sense Voltage High"
            }
            DataBox {
                title: "5V Sense Voltage Low"
            }
            DataBox {
                title: "5V Sense Voltage High"
            }
            DataBox {
                title: "12V Sense Voltage Low"
            }
            DataBox {
                title: "12V Sense Voltage High"
            }
            DataBox {
                title: "DC Bus Voltage Low"
            }
            DataBox {
                title: "DC Bus Voltage High"
            }

            SectionHeader {
                title: "Temperature"
            }
            DataBox {
                title: "Module Temperature Low"
            }
            DataBox {
                title: "Module Temperature High"
            }
            DataBox {
                title: "Control PCB Temperature Low"
            }
            DataBox {
                title: "Control PCB Temperature High"
            }
            DataBox {
                title: "Gate Drive PCB Temperature Low"
            }
            DataBox {
                title: "Gate Drive PCB Temperature High"
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
                title: "Accelerator Shorted"
            }
            DataBox {
                title: "Accelerator Open"
            }

            SectionHeader {
                title: "Brake"
            }
            DataBox {
                title: "Brake Shorted"
            }
            DataBox {
                title: "Brake Open"
            }

            SectionHeader {
                title: "EEPROM"
            }
            DataBox {
                title: "EEPROM Checksum Invalid"
            }
            DataBox {
                title: "EEPROM Data Out of Range"
            }
            DataBox {
                title: "EEPROM Update Required"
            }

            SectionHeader {
                title: "Other"
            }
            DataBox {
                title: "Hardware Gate/Desaturation Fault"
            }
            DataBox {
                title: "HW Over-current Fault"
            }
            DataBox {
                title: "Pre-charge Timeout"
            }
            DataBox {
                title: "Pre-charge Voltage Failure"
            }
            DataBox {
                title: "Hardware DC Bus Over-Voltage"
            }
            DataBox {
                title: "Gate Driver Initialization"
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