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

        function onNewStateOfCharge(value) {bms_state_of_charge.value = value}
        function onNewAmphours(value) {bms_amphours.value = value}
        function onNewResistance(value) {bms_resistance.value = value}
        function onNewHealth(value) {bms_health.value = value}
        function onNewTotalPackCycles(value) {bms_total_pack_cycles.value = value}
        function onNewPackPowerKW(value) {bms_pack_power_kw.value = value}
    }

    Connections {
        target: EnergyMeter

    }

    Connections {
        target: MotorController

        function onNewPWMFrequency(internal_state) {mc_pwm_frequency.value = internal_state}
        function onNewInverterState(internal_state) {mc_inverter_state.value = internal_state}
        function onNewInverterRunMode(internal_state) {mc_inverter_run_mode.value = internal_state}
        function onNewInverterActiveDischargeState(internal_state) {mc_inverter_active_discharge_state.value = internal_state}
        function onNewInverterEnableLockout(internal_state) {mc_inverter_enable_lockout.value = internal_state}
        function onNewBMSActive(internal_state) {mc_bms_active.value = internal_state}
        function onNewBMSLimitingTorque(internal_state) {mc_bms_limiting_torque.value = internal_state}
        function onNewLimitMaxSpeed(internal_state) {mc_limit_max_speed.value = internal_state}
        function onNewLimitHotSpot(internal_state) {mc_limit_hot_spot.value = internal_state}
        function onNewLowSpeedLimiting(internal_state) {mc_low_speed_limiting.value = internal_state}
        function onNewCoolantTempLimiting(internal_state) {mc_coolant_temp_limiting.value = internal_state}

        function onNewAnalogInput1(analog_voltage) {mc_analog_input_1.value = analog_voltage}
        function onNewAnalogInput2(analog_voltage) {mc_analog_input_2.value = analog_voltage}
        function onNewAnalogInput3(analog_voltage) {mc_analog_input_3.value = analog_voltage}
        function onNewAnalogInput4(analog_voltage) {mc_analog_input_4.value = analog_voltage}
        function onNewAnalogInput5(analog_voltage) {mc_analog_input_5.value = analog_voltage}
        function onNewAnalogInput6(analog_voltage) {mc_analog_input_6.value = analog_voltage}

        function onNewTorqueShudder(torque) {mc_torque_shudder.value = torque}
        function onNewCommandedTorque(torque) {mc_commanded_torque.value = torque}
        function onNewTorqueFeedback(torque) {mc_torque_feedback.value = torque}

        function onNewMotorSpeed(speed) {mc_motor_speed.value = speed}
    }
}