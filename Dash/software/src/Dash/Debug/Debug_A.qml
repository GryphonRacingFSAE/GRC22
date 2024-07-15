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
                title: "[Current]  BMS"
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
                title: "[Current]  Energy Meter"
            }
            DataBox {
                id: em_current
                title: "Current"
            }

            SectionHeader {
                title: "[Current]  Motor Controller"
            }
            DataBox {
                id: mc_dc_bus_current
                title: "DC Bus Current"
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
                title: "[Voltage]  BMS"
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

            SectionHeader {
                title: "[Voltage]  Energy Meter"
            }
            DataBox {
                id: em_voltage
                title: "Voltage"
            }

            SectionHeader {
                title: "[Voltage]  Motor Controller"
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

            SectionHeader {
                title: "[Temp]  BMS"
            }
            DataBox {
                id: bms_avg_temp
                title: "Average Temp"
            }
            DataBox {
                id: bms_lowest_temp
                title: "Lowest Temp"
            }
            DataBox {
                id: bms_highest_temp
                title: "Highest Temp"
            }
            DataBox {
                id: bms_heatsink_temp
                title: "Heatsink Temp"
            }
            DataBox {
                id: bms_hottest_thermistor_id
                title: "Hottest Thermistor ID"
            }

            SectionHeader {
                title: "[Temp]  Motor Controller"
            }
            DataBox {
                id: mc_module_a_temp
                title: "Module A Temp"
            }
            DataBox {
                id: mc_module_b_temp
                title: "Module B Temp"
            }
            DataBox {
                id: mc_module_c_temp
                title: "Module C Temp"
            }
            DataBox {
                id: mc_gate_driver_board_temp
                title: "Gate Driver Board Temp"
            }
            DataBox {
                id: mc_control_board_temp
                title: "Control Board Temp"
            }
            DataBox {
                id: mc_coolant_temp
                title: "Coolant Temp"
            }
            DataBox {
                id: mc_hot_spot_temp
                title: "Hot Spot Temp"
            }
            DataBox {
                id: mc_motor_temp
                title: "Motor Temp"
            }
        }
    }

    Connections {
        target: BMS

        function onNewAvgPackCurrent(current) {bms_avg_pack_current.value = current}
        function onNewSignedCurrent(current) {bms_signed_current.value = current}
        function onNewUnsignedCurrent(current) {bms_unsigned_current.value = current}
        function onNewCurrentLimitStatus(current) {bms_current_limit_status.value = current}
        function onNewChargeCurrentLimitA(current) {bms_charge_current_limit_a.value = current}
        function onNewChargeCurrentLimitKW(current) {bms_charge_current_limit_kw.value = current}
        function onNewDischargeCurrentLimitA(current) {bms_discharge_current_limit_a.value = current}
        function onNewDischargeCurrentLimitKW(current) {bms_discharge_current_limit_kw.value = current}

        function onNewVoltage(voltage) {bms_voltage.value = voltage}
        function onNewOpenVoltage(voltage) {bms_voltage.value = voltage}
        function onNewSummedVoltage(voltage) {bms_voltage.value = voltage}
        function onNewAvgCellVoltage(voltage) {bms_avg_cell_voltage.value = voltage}
        function onNewLowCellVoltage(voltage) {bms_low_cell_voltage.value = voltage}
        function onNewLowCellVoltageID(voltage) {bms_low_cell_voltage_id.value = voltage}
        function onNewHighCellVoltage(voltage) {bms_high_cell_voltage.value = voltage}
        function onNewHighCellVoltageID(voltage) {bms_high_cell_voltage_id.value = voltage}
        function onNewAvgCellOpenVoltage(voltage) {bms_avg_cell_open_voltage.value = voltage}
        function onNewLowCellOpenVoltage(voltage) {bms_low_cell_open_voltage.value = voltage}
        function onNewHighCellOpenVoltage(voltage) {bms_high_cell_open_voltage.value = voltage}
    
        function onNewAvgTemp(temp) {bms_avg_temp.value = temp}
        function onNewLowestTemp(temp) {bms_lowest_temp.value = temp}
        function onNewHighestTemp(temp) {bms_highest_temp.value = temp}
        function onNewHeatsinkTemp(temp) {bms_heatsink_temp.value = temp}
        function onNewHottestThermistorID(temp) {bms_hottest_thermistor_id.value = temp}
    }

    Connections {
        target: EnergyMeter

        function onNewCurrent(current) {em_current.value = current}
        function onNewVoltage(voltage) {em_voltage.value = voltage}
    }

    Connections {
        target: MotorController

        function onNewDCBusCurrent(current) {mc_dc_bus_current.value = current}
        
        function onNewDCBusVoltage(voltage) {mc_dc_bus_voltage.value = voltage}
        function onNewOutputVoltage(voltage) {mc_output_voltage.value = voltage}

        function onNewModuleATemp(temp) {mc_module_a_temp.value = temp}
        function onNewModuleBTemp(temp) {mc_module_b_temp.value = temp}
        function onNewModuleCTemp(temp) {mc_module_c_temp.value = temp}
        function onNewGateDriverBoardTemp(temp) {mc_gate_driver_board_temp.value = temp}
        function onNewControlBoardTemp(temp) {mc_control_board_temp.value = temp}
        function onNewCoolantTemp(temp) {mc_coolant_temp.value = temp}
        function onNewHotSpotTemp(temp) {mc_hot_spot_temp.value = temp}
        function onNewMotorTemp(temp) {mc_motor_temp.value = temp}
    }
}