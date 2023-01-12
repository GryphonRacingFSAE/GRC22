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
                title: "Energy Meter"
            }
            DataBox {
                title: "Current"
            }
            DataBox {
                title: "Voltage"
            }
            SectionHeader {
                title: "BMS"
            }
            DataBox {
                title: "Current"
            }
            DataBox {
                title: "Open Voltage"
            }
            DataBox {
                title: "State of Charge"
            }
            DataBox {
                title: "Instantaneous Voltage"
            }
            DataBox {
                title: "High Temperature"
            }
            DataBox {
                title: "Internal Temperature"
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
                title: "Motor Controller  (IGBT Temperature)"
            }
            DataBox {
                title: "Module A Temperature"
            }
            DataBox {
                title: "Module B Temperature"
            }
            DataBox {
                title: "Module C Temperature"
            }
            SectionHeader {
                title: "Motor Controller  (RTD Temperature)"
            }
            DataBox {
                title: "RTD #1 Temperature"
            }
            DataBox {
                title: "RTD #2 Temperature"
            }
            DataBox {
                title: "RTD #3 Temperature"
            }
            DataBox {
                title: "RTD #4 Temperature"
            }
            DataBox {
                title: "RTD #5 Temperature"
            }
            SectionHeader {
                title: "Motor Controller  (Miscellaneous)"
            }
            DataBox {
                title: "Gate Driver Board Temperature"
            }
            DataBox {
                title: "Control Board Temperature"
            }
            DataBox {
                title: "Motor Temperature"
            }
        }
    }
}