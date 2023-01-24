import QtQuick

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
                title: "Motor Controller  (Current)"
            }
            DataBox {
                title: "Phase A Current"
            }
            DataBox {
                title: "Phase B Current"
            }
            DataBox {
                title: "Phase C Current"
            }
            DataBox {
                title: "DC Bus Current"
            }
            SectionHeader {
                title: "Motor Controller  (Voltage)"
            }
            DataBox {
                title: "DC Bus Voltage"
            }
            DataBox {
                title: "Output Voltage"
            }
            DataBox {
                title: "VAB_Vd_Voltage"
            }
            DataBox {
                title: "VBC_Vq_Voltage"
            }
            SectionHeader {
                title: "Motor Controller  (Analog Inputs)"
            }
            DataBox {
                title: "Analog Input #1 Voltage"
            }
            DataBox {
                title: "Analog Input #2 Voltage"
            }
            DataBox {
                title: "Analog Input #3 Voltage"
            }
            DataBox {
                title: "Analog Input #4 Voltage"
            }
            DataBox {
                title: "Analog Input #5 Voltage"
            }
            DataBox {
                title: "Analog Input #6 Voltage"
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
            leftPadding: 24

            SectionHeader {
                title: "Motor Controller  (Motor Position)"
            }
            DataBox {
                title: "Motor Angle"
            }
            DataBox {
                title: "Motor Speed"
            }
            DataBox {
                title: "Electrical Output Frequency"
            }
            DataBox {
                title: "Delta Resolver Filtered"
            }
            SectionHeader {
                title: "Motor Controller  (Flux)"
            }
            DataBox {
                title: "Flux Command"
            }
            DataBox {
                title: "Flux Feedback"
            }
            DataBox {
                title: "Id Feedback"
            }
            DataBox {
                title: "Iq Feedback"
            }
            SectionHeader {
                title: "Motor Controller  (Other)"
            }
            DataBox {
                title: "Torque Shudder"
            }
        }
    }

    Connections {
        target: MotorController
    }
}