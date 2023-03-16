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
                title: "[POST Faults]  Current"
            }
            DataBox {
                id: current_sensor_low
                is_fault: true
                title: "Current Sensor Low"
            }
            DataBox {
                id: current_sensor_high
                is_fault: true
                title: "Current Sensor High"
            }

            SectionHeader {
                title: "[POST Faults]  Voltage"
            }
            DataBox {
                id: sense_voltage_low_1_5_V
                is_fault: true
                title: "1.5V Sense Voltage Low"
            }
            DataBox {
                id: sense_voltage_high_1_5_V
                is_fault: true
                title: "1.5V Sense Voltage High"
            }
            DataBox {
                id: sense_voltage_low_2_5_V
                is_fault: true
                title: "2.5V Sense Voltage Low"
            }
            DataBox {
                id: sense_voltage_high_2_5_V
                is_fault: true
                title: "2.5V Sense Voltage High"
            }
            DataBox {
                id: sense_voltage_low_5_V
                is_fault: true
                title: "5V Sense Voltage Low"
            }
            DataBox {
                id: sense_voltage_high_5_V
                is_fault: true
                title: "5V Sense Voltage High"
            }
            DataBox {
                id: sense_voltage_low_12_V
                is_fault: true
                title: "12V Sense Voltage Low"
            }
            DataBox {
                id: sense_voltage_high_12_V
                is_fault: true
                title: "12V Sense Voltage High"
            }
            DataBox {
                id: dc_bus_voltage_low
                is_fault: true
                title: "DC Bus Voltage Low"
            }
            DataBox {
                id: dc_bus_voltage_high
                is_fault: true
                title: "DC Bus Voltage High"
            }

            SectionHeader {
                title: "[POST Faults]  Temp"
            }
            DataBox {
                id: module_temp_low
                is_fault: true
                title: "Module Temp Low"
            }
            DataBox {
                id: module_temp_high
                is_fault: true
                title: "Module Temp High"
            }
            DataBox {
                id: control_pcb_temp_low
                is_fault: true
                title: "Control PCB Temp Low"
            }
            DataBox {
                id: control_pcb_temp_high
                is_fault: true
                title: "Control PCB Temp High"
            }
            DataBox {
                id: gate_drive_pcb_temp_low
                is_fault: true
                title: "Gate Drive PCB Temp Low"
            }
            DataBox {
                id: gate_drive_pcb_temp_high
                is_fault: true
                title: "Gate Drive PCB Temp High"
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
                title: "[POST Faults]  Accelerator"
            }
            DataBox {
                id: accelerator_shorted
                is_fault: true
                title: "Accelerator Shorted"
            }
            DataBox {
                id: accelerator_open
                is_fault: true
                title: "Accelerator Open"
            }

            SectionHeader {
                title: "[POST Faults]  Brake"
            }
            DataBox {
                id: brake_shorted
                is_fault: true
                title: "Brake Shorted"
            }
            DataBox {
                id: brake_open
                is_fault: true
                title: "Brake Open"
            }

            SectionHeader {
                title: "[POST Faults]  EEPROM"
            }
            DataBox {
                id: eeprom_checksum_invalid
                is_fault: true
                title: "EEPROM Checksum Invalid"
            }
            DataBox {
                id: eeprom_data_out_of_range
                is_fault: true
                title: "EEPROM Data Out of Range"
            }
            DataBox {
                id: eeprom_update_required
                is_fault: true
                title: "EEPROM Update Required"
            }

            SectionHeader {
                title: "[POST Faults]  Other"
            }
            DataBox {
                id: hardware_gate_desaturation_fault
                is_fault: true
                title: "Hardware Gate/Desaturation Fault"
            }
            DataBox {
                id: hw_over_current_fault
                is_fault: true
                title: "HW Over-current Fault"
            }
            DataBox {
                id: pre_charge_timeout
                is_fault: true
                title: "Pre-charge Timeout"
            }
            DataBox {
                id: pre_charge_voltage_failure
                is_fault: true
                title: "Pre-charge Voltage Failure"
            }
            DataBox {
                id: hardware_dc_bus_over_voltage
                is_fault: true
                title: "Hardware DC Bus Over-Voltage"
            }
            DataBox {
                id: gate_driver_initialization
                is_fault: true
                title: "Gate Driver Initialization"
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
        
        function onNewPOSTFaultHigh(post_fault) { // TODO: sort high vs. low + decode input
            current_sensor_low.value = post_fault
            current_sensor_high.value = post_fault

            sense_voltage_low_1_5_V.value = post_fault
            sense_voltage_high_1_5_V.value = post_fault
            sense_voltage_low_2_5_V.value = post_fault
            sense_voltage_high_2_5_V.value = post_fault
            sense_voltage_low_5_V.value = post_fault
            sense_voltage_high_5_V.value = post_fault
            sense_voltage_low_12_V.value = post_fault
            sense_voltage_high_12_V.value = post_fault
            dc_bus_voltage_low.value = post_fault
            dc_bus_voltage_high.value = post_fault

            module_temp_low.value = post_fault
            module_temp_high.value = post_fault
            control_pcb_temp_low.value = post_fault
            control_pcb_temp_high.value = post_fault
            gate_drive_pcb_temp_low.value = post_fault
            gate_drive_pcb_temp_high.value = post_fault

            accelerator_shorted.value = post_fault
            accelerator_open.value = post_fault

            brake_shorted.value = post_fault
            brake_open.value = post_fault

            eeprom_checksum_invalid.value = post_fault
            eeprom_data_out_of_range.value = post_fault
            eeprom_update_required.value = post_fault

            hardware_gate_desaturation_fault.value = post_fault
            hw_over_current_fault.value = post_fault
            pre_charge_timeout.value = post_fault
            pre_charge_voltage_failure.value = post_fault
            hardware_dc_bus_over_voltage.value = post_fault
            gate_driver_initialization.value = post_fault
        }

        function onNewPOSTFaultLow(post_fault) {
        }
    }
}