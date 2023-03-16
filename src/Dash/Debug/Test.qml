import QtQuick

Rectangle {
    width: parent.width
    height: parent.height

    ListModel {
        id: list_model

        // SECTION 1
        ListElement {
            section: "Section 1"
            title: "Data 1A"
            value: ""
        }
        ListElement {
            section: "Section 1"
            title: "Data 1B"
            value: ""
        }
        ListElement {
            section: "Section 1"
            title: "Data 1C"
            value: ""
        }
        
        // SECTION 2
        ListElement {
            section: "Section 2"
            title: "Data 2A"
            value: ""
        }
        ListElement {
            section: "Section 2"
            title: "Data 2B"
            value: ""
        }
        ListElement {
            section: "Section 2"
            title: "Data 2C"
            value: ""
        }

        // SECTION 3
        ListElement {
            section: "Section 3"
            title: "Data 3A"
            value: ""
        }
        ListElement {
            section: "Section 3"
            title: "Data 3B"
            value: ""
        }
        ListElement {
            section: "Section 3"
            title: "Data 3C"
            value: ""
        }
    }

    ListView {
        anchors.fill: parent

        model: list_model

        section {
            property: "section"
            delegate: Column {
                required property string section

                width: parent.width
                height: childrenRect.height

                Text {
                    id: text
                    text: section
                    font.bold: true
                    font.pointSize: 20
                }
                Rectangle {
                    width: parent.width
                    height: 2
                    color: "black"
                }
                Rectangle {
                    width: parent.width
                    height: 4
                    color: "white"
                }
            }
        }

        delegate: Text {
            required property string title
            required property string value

            text: title + ": " + value
            font.pointSize: 10
        }
    }
}