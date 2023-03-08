import QtQuick
import CAN.BMS
import CAN.EnergyMeter
import CAN.MotorController

Item {
    Keys.onDeletePressed: MotorController.clearFaultCodes()
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
                title: "[RUN Faults]  Current"
            }
            DataBox {
                id: over_current_fault
                is_fault: true
                title: "Over-current Fault"
            }
            DataBox {
                id: hardware_over_current_fault
                is_fault: true
                title: "Hardware Over-current Fault"
            }
            DataBox {
                id: current_sensor_fault
                is_fault: true
                title: "Current Sensor Fault"
            }

            SectionHeader {
                title: "[RUN Faults]  Voltage"
            }
            DataBox {
                id: over_voltage_fault
                is_fault: true
                title: "Over-voltage Fault"
            }
            DataBox {
                id: under_voltage_fault
                is_fault: true
                title: "Under-voltage Fault"
            }
            DataBox {
                id: gate_driver_over_voltage
                is_fault: true
                title: "Gate Driver Over-Voltage"
            }
            DataBox {
                id: hardware_dc_bus_over_voltage_fault
                is_fault: true
                title: "Hardware DC Bus Over-voltage Fault"
            }

            SectionHeader {
                title: "[RUN Faults]  Temperature"
            }
            DataBox {
                id: inverter_over_temp_fault
                is_fault: true
                title: "Inverter Over-temperature Fault"
            }
            DataBox {
                id: motor_over_temp_fault
                is_fault: true
                title: "Motor Over-temperature Fault"
            }
            DataBox {
                id: pcb_over_temp_fault
                is_fault: true
                title: "PCB Over-temperature Fault"
            }
            DataBox {
                id: module_a_over_temp_fault
                is_fault: true
                title: "Module A Over-temperature Fault"
            }
            DataBox {
                id: module_b_over_temp_fault
                is_fault: true
                title: "Module B Over-temperature Fault"
            }
            DataBox {
                id: module_c_over_temp_fault
                is_fault: true
                title: "Module C Over-temperature Fault"
            }
            DataBox {
                id: gate_drive_board_1_over_temp_fault
                is_fault: true
                title: "Gate Drive Board 1 Over-temperature Fault"
            }
            DataBox {
                id: gate_drive_board_2_over_temp_fault
                is_fault: true
                title: "Gate Drive Board 2 Over-temperature Fault"
            }
            DataBox {
                id: gate_drive_board_3_over_temp_fault
                is_fault: true
                title: "Gate Drive Board 3 Over-temperature Fault"
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
                title: "[RUN Faults]  Accelerator"
            }
            DataBox {
                id: accelerator_input_shorted_fault
                is_fault: true
                title: "Accelerator Input Shorted Fault"
            }
            DataBox {
                id: accelerator_input_open_fault
                is_fault: true
                title: "Accelerator Input Open Fault"
            }

            SectionHeader {
                title: "[RUN Faults]  Brake"
            }
            DataBox {
                id: brake_input_shorted_fault
                is_fault: true
                title: "Brake Input Shorted Fault"
            }
            DataBox {
                id: brake_input_open_fault
                is_fault: true
                title: "Brake Input Open Fault"
            }

            SectionHeader {
                title: "[RUN Faults]  Other"
            }
            DataBox {
                id: motor_over_speed_fault
                is_fault: true
                title: "Motor Over-speed Fault"
            }
            DataBox {
                id: direction_command_fault
                is_fault: true
                title: "Direction Command Fault"
            }
            DataBox {
                id: inverter_response_time_out_fault
                is_fault: true
                title: "Inverter Response Time-out Fault"
            }
            DataBox {
                id: hardware_gate_desaturation_fault
                is_fault: true
                title: "Hardware Gate/Desaturation Fault"
            }
            DataBox {
                id: can_command_message_lost_fault
                is_fault: true
                title: "CAN Command Message Lost Fault"
            }
            DataBox {
                id: resolver_not_connected
                is_fault: true
                title: "Resolver Not Connected"
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

        function onNewTestFault(value) {
            over_current_fault.value = value
            hardware_over_current_fault.value = value
            current_sensor_fault.value = value

            over_voltage_fault.value = value
            under_voltage_fault.value = value
            gate_driver_over_voltage.value = value
            hardware_dc_bus_over_voltage_fault.value = value

            inverter_over_temp_fault.value = value
            motor_over_temp_fault.value = value
            pcb_over_temp_fault.value = value
            module_a_over_temp_fault.value = value
            module_b_over_temp_fault.value = value
            module_c_over_temp_fault.value = value
            gate_drive_board_1_over_temp_fault.value = value
            gate_drive_board_2_over_temp_fault.value = value
            gate_drive_board_3_over_temp_fault.value = value

            accelerator_input_shorted_fault.value = value
            accelerator_input_open_fault.value = value

            brake_input_shorted_fault.value = value
            brake_input_open_fault.value = value

            motor_over_speed_fault.value = value
            direction_command_fault.value = value
            inverter_response_time_out_fault.value = value
            hardware_gate_desaturation_fault.value = value
            can_command_message_lost_fault.value = value
            resolver_not_connected.value = value
        }
    }
}