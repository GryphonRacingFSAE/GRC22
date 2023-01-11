import QtQuick
import QtQuick.Controls

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

    readonly property var display_sources: ["Drive/Drive.qml", "Debug/Debug.qml", "Tuning/Tuning.qml"];
    property int selected_display: 0;
    MouseArea {
        anchors.fill: parent
        onClicked: () => {
            active_dash.source = display_sources[++selected_display % display_sources.length];
        }
    }
}