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
            leftPadding: 16

            SectionHeader {
                title: "BMS"
            }
            DataBox {
                id: bms_avg_pack_current
                title: "Average Pack Current"
            }
            DataBox {
                id: bms_signed_current
                title: "Signed Current"
            }
            DataBox {
                id: bms_unsigned_current
                title: "Unsigned Current"
            }
            DataBox {
                id: bms_current_limit_status
                title: "Current Limit Status"
            }
            DataBox {
                id: bms_charge_current_limit_a
                title: "Charge Current Limit (A)"
            }
            DataBox {
                id: bms_charge_current_limit_kw
                title: "Charge Current Limit (kW)"
            }
            DataBox {
                id: bms_discharge_current_limit_a
                title: "Discharge Current Limit (A)"
            }
            DataBox {
                id: bms_discharge_current_limit_kw
                title: "Discharge Current Limit (kW)"
            }

            SectionHeader {
                title: "Energy Meter"
            }
            DataBox {
                id: em_current
                title: "Current"
            }

            SectionHeader {
                title: "Motor Controller"
            }
            DataBox {
                id: mc_dc_bus_current
                title: "DC Bus Current"
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
            leftPadding: 8

            SectionHeader {
                title: "BMS"
            }
            DataBox {
                id: bms_voltage
                title: "Voltage"
            }
            DataBox {
                id: bms_open_voltage
                title: "Open Voltage"
            }
            DataBox {
                id: bms_summed_voltage
                title: "Summed Voltage"
            }
            DataBox {
                id: bms_avg_cell_voltage
                title: "Average Cell Voltage"
            }
            DataBox {
                id: bms_low_cell_voltage
                title: "Low Cell Voltage"
            }
            DataBox {
                id: bms_low_cell_voltage_id
                title: "Low Cell Voltage ID"
            }
            DataBox {
                id: bms_high_cell_voltage
                title: "High Cell Voltage"
            }
            DataBox {
                id: bms_high_cell_voltage_id
                title: "High Cell Voltage ID"
            }
            DataBox {
                id: bms_avg_cell_open_voltage
                title: "Average Cell Open Voltage"
            }
            DataBox {
                id: bms_low_cell_open_voltage
                title: "Low Cell Open Voltage"
            }
            DataBox {
                id: bms_high_cell_open_voltage
                title: "High Cell Open Voltage"
            }
            DataBox {
                id: bms_min_cell_voltage
                title: "Minimum Cell Voltage"
            }
            DataBox {
                id: bms_max_cell_voltage
                title: "Maximum Cell Voltage"
            }

            SectionHeader {
                title: "Energy Meter"
            }
            DataBox {
                id: em_voltage
                title: "Voltage"
            }

            SectionHeader {
                title: "Motor Controller"
            }
            DataBox {
                id: mc_dc_bus_voltage
                title: "DC Bus Voltage"
            }
            DataBox {
                id: mc_output_voltage
                title: "Output Voltage"
            }
        }
    }

    Connections {
        target: BMS

        function onNewTestValue(value) {
            bms_avg_pack_current.value = value
            bms_signed_current.value = value
            bms_unsigned_current.value = value
            bms_current_limit_status.value = value
            bms_charge_current_limit_a.value = value
            bms_charge_current_limit_kw.value = value
            bms_discharge_current_limit_a.value = value
            bms_discharge_current_limit_kw.value = value

            bms_voltage.value = value
            bms_open_voltage.value = value
            bms_summed_voltage.value = value
            bms_avg_cell_voltage.value = value
            bms_low_cell_voltage.value = value
            bms_low_cell_voltage_id.value = value
            bms_high_cell_voltage.value = value
            bms_high_cell_voltage_id.value = value
            bms_avg_cell_open_voltage.value = value
            bms_low_cell_open_voltage.value = value
            bms_high_cell_open_voltage.value = value
            bms_min_cell_voltage.value = value
            bms_max_cell_voltage.value = value
        }
    }

    Connections {
        target: EnergyMeter
        
        function onNewTestValue(value) {
            em_current.value = value
            em_voltage.value = value
        }
    }

    Connections {
        target: MotorController

        function onNewTestValue(value) {
            mc_dc_bus_current.value = value
            mc_dc_bus_voltage.value = value
            mc_output_voltage.value = value
        }
    }
}