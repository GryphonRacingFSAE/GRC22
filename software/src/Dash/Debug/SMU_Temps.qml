import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CAN.SMU

Rectangle {
    id: root
    color: "black"
    
    readonly property int segments: 5
    readonly property int rows: 10
    readonly property int columns: 28
    readonly property int boxSize: 25

    Rectangle {
        id: box_matrix

        color: "white"
        anchors.centerIn: parent
        width: root.columns * boxSize
        height: root.rows * boxSize
        GridLayout {
            anchors.centerIn: parent
            width: root.columns * boxSize
            height: root.rows * boxSize

            rows: root.rows
            columns: root.columns

            columnSpacing: 0
            rowSpacing: 0
            Repeater {
                id: repeater
                model: SMU.temperatures
                Rectangle {
                    property int torque: modelData

                    Text {
                        font.family: "Consolas"
                        width: parent.width
                        height: parent.height

                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        text: repeater.itemAt(index).torque ?? "NULL"
                        font.pointSize: 10
                    }

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: {
                        if (torque < 0) {
                            return Qt.rgba(0, 0, 1, torque/-40)
                        } else {
                            return Qt.rgba(1, 0, 0, torque/80)
                        }
                    }
                }
            }
        }
    }
}