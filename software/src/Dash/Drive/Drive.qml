import QtQuick
import QtQuick.Layouts
import CAN.MotorController
import CAN.BMS
import CAN.VCU
import CAN.EnergyMeter

Item {
    id: root
    RowLayout {
        id: main
        anchors {
            top: test.bottom
            bottom: root.bottom
            left: parent.left
            right: parent.right
            bottomMargin: 30
        }



        //Motor and Inverter
        ColumnLayout {
            Layout.bottomMargin: root.height/10
            Layout.preferredWidth: parent.width/16*5

            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: motorTempBox
                title: "Motor Temp"
                fontSize: root.height/20
                precision: 0;
                low: 30
                high: 75
            }

            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: inverterTempBox
                title: "Inverter Temp"
                fontSize: root.height/20
                precision: 0;
                low: 20
                high: 40
            }
        }


        //Speed and Accum Temp
        ColumnLayout {
            Layout.preferredWidth: parent.width/16*5
            Layout.bottomMargin: root.height/10

            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: speedBox
                title: "Speed"
                fontSize: root.height/20
                precision: 0
            }

            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: accumTempBox
                title: "Accum Temp"
                fontSize: root.height/20
                precision: 0;
                low: 25
                high: 60
            }
        }

        //Oil and Coolant
        ColumnLayout {
            Layout.preferredWidth: parent.width/16*5

            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: oilTempBox
                title: "Oil Temp"
                fontSize: root.height/20
                precision: 0;
                low: 30
                high: 70
            }
            DataBox {
                Layout.fillHeight: true
                Layout.fillWidth: true
                id: coolantTempBox
                title: "Coolant Temp"
                fontSize: root.height/20
                precision: 0;
                low: 25
                high: 60
            }   
            Layout.bottomMargin: root.height/10
        }

        //Speed
        ColumnLayout {
            Layout.bottomMargin: root.height/10
            Layout.preferredWidth: parent.width/16*4
            anchors.verticalCenter: parent.verticalCenter

            Rectangle{
                id: rightPanel
                width: 200
                height: 300
                color: "white"
                // pedal position displays
                RowLayout{
                    spacing: 10
                    anchors.verticalCenter: parent.verticalCenter
                    PedalPosBox{
                        id: brakeBox
                        width: 60
                        height: 300
                        borderWidth:2
                        lineColour: "red"
                        letter: "B"
                        value: 0
                    }
                     PedalPosBox{
                        id: acceleratorBox
                        width: 60
                        height: 300
                        borderWidth:2
                        lineColour: "green"
                        letter: "A"
                        value: 0
                    }
                    //tick marks
                    Rectangle{
                        width: 60
                        height: 300
                        color: "white"
                        // border{
                        //     width: 2
                        //     color: "white"
                        // }
                        Repeater{
                            id: ticks
                            model: 11
                            Text{
                                text: `${(index)*10}`+"%"
                                font.family: "Consolas"
                                font.pointSize: 10
                                font.bold: true
                                color: "black"
                                anchors{
                                    left: parent.left
                                    bottom: parent.bottom
                                    bottomMargin: (parent.height) * (index)*10/100 - 7
                                }
                            }
                        }             
                    }
                }
            }
        }

    }

    RowLayout {
        id: test
        height: root.height/5
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 10
        }
        Battery {
            id: battery_bar
            Layout.fillHeight: true
            Layout.fillWidth: true
        }
        ColumnLayout {    
            RowLayout {
                Indicator {
                    id: pump_active
                    name: "PUMP"
                    color: "green"
                }
                Indicator {
                    id: acc_fan_active
                    name: "ACC FAN"
                    color: "green"
                }
                Indicator {
                    id: rad_fan_active
                    name: "RAD FAN"
                    color: "green"
                }
                Indicator {
                    id: brake_switch
                    name: "BRAKE"
                    color: "green"
                }
                Indicator {
                    id: rtd_button
                    name: "RTD BUT"
                    color: "green"
                }
            }
            RowLayout {
                Indicator {
                    id: rtd_invalid
                    name: "RTD"
                    color: "green"
                }
                Indicator {
                    id: bspc_invalid
                    name: "BSPC"
                    color: "red"
                }
                Indicator {
                    id: apps_conflict
                    name: ">10%"
                    color: "red"
                }
                Indicator {
                    id: apps_oor    
                    name: "A OOR"
                    color: "red"
                }
                Indicator {
                    id: brake_oor
                    name: "B OOR"
                    color: "red"
                }
            }
        }


    }

    Connections {
        target: MotorController
        function onNewMotorSpeed(speed) {
            let gear_ratio = 1/3.48; // 3.48:1 gear ratio
            let wheel_rpm = speed * gear_ratio;
            let wheel_diameter_inch = 16; // 16" OD
            let wheel_circumfrence_m = wheel_diameter_inch * 0.0254 * 3.14; // inch -> m = inch * 0.0254
            let wheel_surface_speed_mpm = wheel_circumfrence_m * wheel_rpm;
            let wheel_surface_speed_kmph = wheel_surface_speed_mpm / 1000 * 60 // m/min -> km/h = m / 1000 * 60,

            speedBox.value = `${wheel_surface_speed_kmph.toFixed(0)}`
        }
        function onNewCoolantTemp(temp) {
            coolantTempBox.value = temp
        }
        function onNewAnalogInput2(temp) { // oil temp
            oilTempBox.value = temp
        }
        function onNewMotorTemp(temp) {
            motorTempBox.value = temp
        }
        function onNewAnalogInput1(voltage) // 12V voltage
        {
        }

        function onNewRequestedTorque(torque)
        {
            if (torque < 0)
            {
                torqueContainer.color = "red"
            }
            else if(torque > 0)
            {
                torqueContainer.color = "green"
            }
            else{
                torqueContainer.color = "grey"
            }

            torqueValue.text = `${torque.toFixed(0)}`
        }

        function onNewOutputTorque(torque) {
            if (torque < 0)
            {
                torqueContainer2.color = "red"
            }
            else if(torque > 0)
            {
                torqueContainer2.color = "green"
            }
            else{
                torqueContainer2.color = "grey"
            }

            torqueValue2.text = `${torque.toFixed(0)}`
        }
    }

    Connections {
        target: VCU
        function onNewBrakePressure(value) {
            brakeBox.value = value/15;
        }
        function onNewAcceleratorPos(percent) {
            acceleratorBox.value = percent;
        }
      
        function onNewBSPCInvalid(state) {
            bspc_invalid.value = state;
        }
        function onNewAPPSOutOfRange(state) {
            apps_oor.value = state;
        }
        function onNewAPPSSensorConflict(state) {
            apps_conflict.value = state;
        }
        function onNewBrakeOutOfRange(state) { 
            brake_oor.value = state;
        }
        function onNewRTDInvalid(state) {
            rtd_invalid.value = state != 1;
        }
        function onNewRTDButton(state) {
            rtd_button.value = state;
        }
        function onNewBrakeSwitch(state) {
            brake_switch.value = state;
        }
        function onNewPumpActive(state) {
            pump_active.value = state;
        }
        function onNewAccumulatorFanActive(state) {
            acc_fan_active.value = state;
        }
        function onNewRadiatorFanActive(state) {
            rad_fan_active.value = state;
        }
    }
    Connections {
        target: BMS
        function onNewStateOfCharge(percent) {
            battery_bar.percent = percent
        }
        function onNewAvgTemp(temp) {}
        function onNewHighestTemp(temp) {
            inverterTempBox.value = temp
            accumTempBox.value = temp
        }
        function onNewAvgPackCurrent(current) {}
        function onNewVoltage(voltage) {}
        function onNewOpenVoltage(voltage){}
    }
}