import QtQuick
import QtQuick.Controls
import CAN.MotorController

ApplicationWindow {
    id: application_window
    visible: true
    width: 800
    height: width * 10/16
    title: "GRC Dash"
    color: "white"
    maximumHeight: height
    maximumWidth: width
    minimumHeight: height
    minimumWidth: width
    flags: Qt.Window // | Qt.FramelessWindowHint add when final project is reached.

    Loader {
        id: active_dash
        source: display_sources[selected_display]
        anchors.fill: parent
    }

    readonly property var display_sources: ["Drive/Drive.qml", "Debug/Debug_A.qml", "Debug/Debug_B.qml", "Debug/Debug_C.qml", "Debug/Debug_D.qml", "Tuning/Tuning.qml"];
    property int selected_display: 0;

    Item {
        anchors.fill: parent
        focus: true

        Keys.onLeftPressed: {
            if (--selected_display < 0) {
                selected_display = display_sources.length - 1;
            }
            active_dash.source = display_sources[selected_display];
        }
        Keys.onRightPressed: {
            if (++selected_display > display_sources.length - 1) {
                selected_display = 0;
            }
            active_dash.source = display_sources[selected_display];
        }
        Keys.onDeletePressed: {
            MotorController.clearFaultCodes();
        }
    }
}