import QtQuick
import CAN.MotorController
import CAN.BMS
import CAN.EnergyMeter

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
                id: em_current
                title: "Current"
            }
            DataBox {
                id: em_voltage
                title: "Voltage"
            }
            SectionHeader {
                title: "BMS"
            }
            DataBox {
                id: bms_current
                title: "Current"
            }
            DataBox {
                id: bms_open_voltage
                title: "Open Voltage"
            }
            DataBox {
                id: bms_state_of_charge
                title: "State of Charge"
            }
            DataBox {
                id: bms_instantaneous_voltage
                title: "Instantaneous Voltage"
            }
            DataBox {
                id: bms_high_temp
                title: "High Temperature"
            }
            DataBox {
                id: bms_internal_temp
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
                id: mc_module_a_temp
                title: "Module A Temperature"
            }
            DataBox {
                id: mc_module_b_temp
                title: "Module B Temperature"
            }
            DataBox {
                id: mc_module_c_temp
                title: "Module C Temperature"
            }
            SectionHeader {
                title: "Motor Controller  (RTD Temperature)"
            }
            DataBox {
                id: mc_rtd_1_temp
                title: "RTD #1 Temperature"
            }
            DataBox {
                id: mc_rtd_2_temp
                title: "RTD #2 Temperature"
            }
            DataBox {
                id: mc_rtd_3_temp
                title: "RTD #3 Temperature"
            }
            DataBox {
                id: mc_rtd_4_temp
                title: "RTD #4 Temperature"
            }
            DataBox {
                id: mc_rtd_5_temp
                title: "RTD #5 Temperature"
            }
            SectionHeader {
                title: "Motor Controller  (Miscellaneous)"
            }
            DataBox {
                id: mc_gate_driver_board_temp
                title: "Gate Driver Board Temperature"
            }
            DataBox {
                id: mc_control_board_temp
                title: "Control Board Temperature"
            }
            DataBox {
                id: mc_motor_temp
                title: "Motor Temperature"
            }
        }
    }

    Connections {
        target: EnergyMeter

        function onNewTestValue(test_value) {
            em_current.value = test_value
            em_voltage.value = test_value
        }
    }

    Connections {
        target: BMS

        function onNewTestValue(test_value) {
            bms_current.value = test_value
            bms_open_voltage.value = test_value
            bms_state_of_charge.value = test_value
            bms_instantaneous_voltage.value = test_value
            bms_high_temp.value = test_value
            bms_internal_temp.value = test_value
        }
    }

    Connections {
        target: MotorController

        function onNewTestValue(test_value) {
            mc_module_a_temp.value = test_value
            mc_module_b_temp.value = test_value
            mc_module_c_temp.value = test_value
            mc_rtd_1_temp.value = test_value
            mc_rtd_2_temp.value = test_value
            mc_rtd_3_temp.value = test_value
            mc_rtd_4_temp.value = test_value
            mc_rtd_5_temp.value = test_value
            mc_gate_driver_board_temp.value = test_value
            mc_control_board_temp.value = test_value
            mc_motor_temp.value = test_value
        }
    }
}