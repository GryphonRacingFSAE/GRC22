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

    readonly property list<string> display_sources: ["Drive/Drive.qml", "Debug/Debug_A.qml", "Debug/Debug_B.qml", "Debug/Debug_C.qml", "Debug/Debug_D.qml", "Tuning/Tuning.qml", "Tuning/TractionControl.qml"];
    property int selected_display: 0;

    Item {
        anchors.fill: parent
        focus: true

        // Navigate between screens using tab & shift+tab
        Keys.onTabPressed: selected_display = (selected_display + 1) % display_sources.length
        Keys.onBacktabPressed: selected_display = (selected_display - 1 + display_sources.length) % display_sources.length
        Keys.forwardTo: [active_dash.item]
    }
}