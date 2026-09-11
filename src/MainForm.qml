import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 1920
    height: 1080
    color: "#121212"

    property alias ipInputText: ipField.text
    property alias portInputText: portField.text
    signal testConnectionClicked()
    signal saveAndStartClicked()

    // Expose root activation mapping to force clean startup focus loops
    Component.onCompleted: ipField.forceActiveFocus()

    Column {
        anchors.centerIn: parent
        spacing: 40
        width: 600

        Text {
            text: "GoodWe Monitor Setup"
            color: "#ffffff"
            font.pixelSize: 48
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            width: parent.width
        }

        Column {
            spacing: 10
            width: parent.width

            Text {
                text: "Inverter IP Address"
                color: "#aaaaaa"
                font.pixelSize: 24
            }

            TextField {
                id: ipField
                width: parent.width
                placeholderText: "e.g. 192.168.1.123"
                font.pixelSize: 28
                color: "#ffffff"
                
                // Strict TV Navigation Mapping: Clicks route downwards to Port
                KeyNavigation.down: portField
                KeyNavigation.tab: portField
                
                background: Rectangle {
                    color: ipField.activeFocus ? "#2c2c2c" : "#1e1e1e"
                    border.color: ipField.activeFocus ? "#007acc" : "#333333"
                    border.width: 2
                    radius: 8
                }
            }
        }

        Column {
            spacing: 10
            width: parent.width

            Text {
                text: "Port"
                color: "#aaaaaa"
                font.pixelSize: 24
            }

            TextField {
                id: portField
                width: parent.width
                text: "502"
                font.pixelSize: 28
                color: "#ffffff"
                
                // Dynamic Key Routing loops between adjacent elements
                KeyNavigation.up: ipField
                KeyNavigation.down: testButton
                KeyNavigation.tab: testButton
                
                background: Rectangle {
                    color: portField.activeFocus ? "#2c2c2c" : "#1e1e1e"
                    border.color: portField.activeFocus ? "#007acc" : "#333333"
                    border.width: 2
                    radius: 8
                }
            }
        }

        Button {
            id: testButton
            width: parent.width
            text: "Test Connection"
            onClicked: root.testConnectionClicked()
            
            KeyNavigation.up: portField
            KeyNavigation.down: saveButton
            KeyNavigation.tab: saveButton

            contentItem: Text {
                text: testButton.text
                font.pixelSize: 24
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: testButton.activeFocus ? "#007acc" : "#252526"
                radius: 8
            }
        }

        Button {
            id: saveButton
            width: parent.width
            text: "Save & Start"
            onClicked: root.saveAndStartClicked()

            // Complete the closed loop context to prevent focus escaping off-screen boundaries
            KeyNavigation.up: testButton
            KeyNavigation.down: ipField 
            KeyNavigation.tab: ipField

            contentItem: Text {
                text: saveButton.text
                font.pixelSize: 24
                color: "#ffffff"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: saveButton.activeFocus ? "#4CAF50" : "#252526"
                radius: 8
            }
        }
    }
}
