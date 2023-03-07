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
                title: "[Misc]  BMS"
            }
            DataBox {
                id: bms_state_of_charge
                title: "State of Charge"
            }
            DataBox {
                id: bms_amphours
                title: "Amphours"
            }
            DataBox {
                id: bms_resistance
                title: "Resistance"
            }
            DataBox {
                id: bms_health
                title: "Health"
            }
            DataBox {
                id: bms_total_pack_cycles
                title: "Total Pack Cycles"
            }
            DataBox {
                id: bms_pack_power_kw
                title: "Pack Power (kW)"
            }

            SectionHeader {
                title: "[Internal]  Motor Controller"
            }
            DataBox {
                id: mc_pwm_frequency
                title: "PWM Frequency"
            }
            DataBox {
                id: mc_inverter_state
                title: "Inverter State"
            }
            DataBox {
                id: mc_inverter_run_mode
                title: "Inverter Run Mode"
            }
            DataBox {
                id: mc_inverter_active_discharge_state
                title: "Inverter Active Discharge State"
            }
            DataBox {
                id: mc_inverter_enable_lockout
                title: "Inverter Enable Lockout"
            }
            DataBox {
                id: mc_bms_active
                title: "BMS Active"
            }
            DataBox {
                id: mc_bms_limiting_torque
                title: "BMS Limiting Torque"
            }
            DataBox {
                id: mc_limit_max_speed
                title: "Limit Max Speed"
            }
            DataBox {
                id: mc_limit_hot_spot
                title: "Limit Hot Spot"
            }
            DataBox {
                id: mc_low_speed_limiting
                title: "Low Speed Limiting"
            }
            DataBox {
                id: mc_coolant_temp_limiting
                title: "Coolant Temperature Limiting"
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
                title: "[Analog]  Motor Controller"
            }
            DataBox {
                id: mc_analog_input_1
                title: "Analog Input #1"
            }
            DataBox {
                id: mc_analog_input_2
                title: "Analog Input #2"
            }
            DataBox {
                id: mc_analog_input_3
                title: "Analog Input #3"
            }
            DataBox {
                id: mc_analog_input_4
                title: "Analog Input #4"
            }
            DataBox {
                id: mc_analog_input_5
                title: "Analog Input #5"
            }
            DataBox {
                id: mc_analog_input_6
                title: "Analog Input #6"
            }

            SectionHeader {
                title: "[Torque]  Motor Controller"
            }
            DataBox {
                id: mc_torque_shudder
                title: "Torque Shudder"
            }
            DataBox {
                id: mc_commanded_torque
                title: "Commanded Torque"
            }
            DataBox {
                id: mc_torque_feedback
                title: "Torque Feedback"
            }

            SectionHeader {
                title: "[Speed]  Motor Controller"
            }
            DataBox {
                id: mc_motor_speed
                title: "Motor Speed"
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

            // SectionHeader {
            //     title: "HEADER"
            // }
            // DataBox {
            //     title: "DATA BOX"
            // }
        }
    }

    Connections {
        target: BMS

        function onNewTestValue(value) {
            bms_state_of_charge.value = value
            bms_amphours.value = value
            bms_resistance.value = value
            bms_health.value = value
            bms_total_pack_cycles.value = value
            bms_pack_power_kw.value = value
        }
    }

    Connections {
        target: EnergyMeter

    }

    Connections {
        target: MotorController

        /*
        function onNewTestValue(value) {
            mc_pwm_frequency.value = value
            mc_inverter_state.value = value
            mc_inverter_run_mode.value = value
            mc_inverter_active_discharge_state.value = value
            mc_inverter_enable_lockout.value = value
            mc_bms_active.value = value
            mc_bms_limiting_torque.value = value
            mc_limit_max_speed.value = value
            mc_limit_hot_spot.value = value
            mc_low_speed_limiting.value = value
            mc_coolant_temp_limiting.value = value

            mc_analog_input_1.value = value
            mc_analog_input_2.value = value
            mc_analog_input_3.value = value
            mc_analog_input_4.value = value
            mc_analog_input_5.value = value
            mc_analog_input_6.value = value

            mc_torque_shudder.value = value
            mc_commanded_torque.value = value
            mc_torque_feedback.value = value

            mc_motor_speed.value = value
        }
        */

        function onNewPWMFrequency(state) {mc_pwm_frequency.value = state}
        function onNewInverterState(state) {mc_inverter_state.value = state}
        function onNewInverterRunMode(state) {mc_inverter_run_mode.value = state}
        function onNewInverterActiveDischargeState(state) {mc_inverter_active_discharge_state.value = state}
        function onNewInverterEnablelockout(state) {mc_inverter_enable_lockout.value = state}
        function onNewBMSActive(state) {mc_bms_active.value = state}
        function onNewBMSLimitingTorque(state) {mc_bms_limiting_torque.value = state}
        function onNewLimitMaxSpeed(state) {mc_limit_max_speed.value = state}
        function onNewLimitHotSpot(state) {mc_limit_hot_spot.value = state}
        function onNewLowSpeedLimiting(state) {mc_low_speed_limiting.value = state}
        function onNewCoolantTempLimiting(state) {mc_coolant_temp_limiting.value = state}

        function onNewAnalogInput1(voltage) {mc_analog_input_1.value = voltage}
        function onNewAnalogInput2(voltage) {mc_analog_input_2.value = voltage}
        function onNewAnalogInput3(voltage) {mc_analog_input_3.value = voltage}
        function onNewAnalogInput4(voltage) {mc_analog_input_4.value = voltage}
        function onNewAnalogInput5(voltage) {mc_analog_input_5.value = voltage}
        function onNewAnalogInput6(voltage) {mc_analog_input_6.value = voltage}

        function onNewTorqueShudder(torque) {mc_torque_shudder.value = torque}
        function onNewCommandedTorque(torque) {mc_commanded_torque.value = torque}
        function onNewTorqueFeedback(torque) {mc_torque_feedback.value = torque}

        function onNewMotorSpeed(speed) {mc_motor_speed.value = speed}
    }
}