import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.AN400ECU
import "Drive"

ApplicationWindow {
    visible: true
    width: 800
    height: width * 10/16
    title: "GRC Dash 2019"
    color: "white"
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    flags: Qt.Window // | Qt.FramelessWindowHint add when final project is reached.
    id: app_window

    RowLayout {
        id: toprow
        height: parent.height/5
        anchors{
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
        //Logo
        Image {
            source:"qrc:/images/Logo"
            Layout.fillHeight: true
            Layout.preferredWidth: sourceSize.width/sourceSize.height * height
        }
    }
    RowLayout {
        id: main
        anchors{
            top: toprow.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            bottomMargin: 30
        }

        // Coolant and Air
        ColumnLayout {
            Layout.preferredWidth: app_window.width/16*5
            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: coolantTempBox
                title: "Coolant Temp"
                fontSize: app_window.height/20
                precision: 1;
                low: 30
                high: 70
            }
            DataBox {
                Layout.fillHeight: true
                Layout.fillWidth: true
                id: airTempBox
                title: "Air Temp"
                fontSize: app_window.height/20
                precision: 1;
                low: 25
                high: 60
            }
        }

        //Speed
        Item {
            Layout.preferredWidth: app_window.width/16*6
            Layout.fillHeight: true
            Text {
                id: speedtext
                text: "kmph"
                font.bold: true
                color:"black"
                font.pointSize: main.height/15
                anchors{
                    top: parent.top
                    horizontalCenter: parent.horizontalCenter
                    topMargin: main.height/15
                }
            }
            Text {
                id: speedValue
                font.pointSize: main.height/3.5
                opacity: 0.9
                font.bold: true
                color:"black"
                text: ""
                anchors{
                    top:speedtext.bottom
                    horizontalCenter: parent.horizontalCenter
                    topMargin: - main.height/15
                }
            }
            DataBox {
                id: lambdaBox
                title: "Lambda"
                fontSize: app_window.height/20
                precision: 1;
                low: 20
                high: 40
                anchors{
                    top:speedValue.bottom
                    horizontalCenter: parent.horizontalCenter
                    topMargin: main.height/8
                }
            }
        }

        //Accum and Motor
        ColumnLayout {
            Layout.preferredWidth: app_window.width/16*5
            
            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: tpsBox
                title: "TPS"
                fontSize: app_window.height/20
                precision: 1;
                low: 20
                high: 40
            }
            DataBox {
                Layout.fillWidth: true
                Layout.fillHeight: true
                id: mapBox
                title: "MAP"
                fontSize: app_window.height/20
                precision: 1;
                low: 30
                high: 75
            }
        }
    }


    Connections {
        target: AN400ECU
        function onNewRPM(rpm) {
            let gear_ratio = 1/3.48; // 3.48:1 gear ratio
            let wheel_rpm = rpm * gear_ratio;
            let wheel_diameter_inch = 16; // 16" OD
            let wheel_circumfrence_m = wheel_diameter_inch * 0.0254 * 3.14; // inch -> m = inch * 0.0254
            let wheel_surface_speed_mpm = wheel_circumfrence_m * wheel_rpm;
            let wheel_surface_speed_kmph = wheel_surface_speed_mpm / 1000 * 60 // m/min -> km/h = m / 1000 * 60, 

            speedValue.text = `${wheel_surface_speed_kmph.toFixed(0)}`
        }
        function onNewCoolantTemp(coolant_temp) {
            coolantTempBox.value = coolant_temp
        }
        function onNewAirTemp(temp) {
            airTempBox.value = temp
        }
        function onNewBatteryVoltage(voltage) {
            let empty_battery = 11; // 11 Volts becomes 0% Battery
            let full_battery = 15;  // 15 Volts becomes 100% Battery
            const clamp = (num, min, max) => Math.min(Math.max(num, min), max);
            // Now contains values from 11v - 15v
            let clamped_voltages = clamp(voltage, empty_battery, full_battery); 
            let percent = (clamped_voltages - empty_battery) * (100 / (full_battery - empty_battery));
            battery_bar.percent = percent
        }
        function onNewTPS(tps) {
            tpsBox.value = tps
        }
        function onNewLambda(lambda) {
            lambdaBox.value = lambda
        }
        function onNewMAP(map) {
            mapBox.value = map
        }
    }
}