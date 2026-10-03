import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 1920
    height: 1080
    color: "#121212"

    property alias ipInputText: ipField.text
    property alias portInputText: portField.text
    property bool dashboardVisible: false
    property string connectionFeedback: ""
    property string connectionFeedbackColor: "#9aa0a8"
    property bool connectionTestRunning: false
    property bool connectionLive: false
    signal testConnectionClicked()
    signal saveAndStartClicked()

    // Expose root activation mapping to force clean startup focus loops
    Component.onCompleted: ipField.forceActiveFocus()

    Connections {
        target: modbusClient
        function onConnectionStateChanged(state) {
            root.connectionLive = state === 2
            if (dashboardLoader.item) dashboardLoader.item.offline = state !== 2
            if (state === 2) {
                connectionTimeout.stop()
                root.connectionTestRunning = false
                root.connectionFeedback = "✓ Connected to inverter"
                root.connectionFeedbackColor = "#70d6a0"
            } else if (state === 0) {
                connectionTimeout.stop()
                root.connectionTestRunning = false
                root.connectionFeedback = "✕ Inverter unreachable"
                root.connectionFeedbackColor = "#ff6b78"
            }
        }
    }

    Connections {
        target: telemetry
        function onTelemetryUpdated() {
            if (!dashboardLoader.item) return
            dashboardLoader.item.solarPower = telemetry.solarPower
            dashboardLoader.item.gridPower = telemetry.gridPower
            dashboardLoader.item.houseLoad = telemetry.houseLoad
            dashboardLoader.item.batteryPower = telemetry.batteryPower
            dashboardLoader.item.batterySoc = telemetry.batterySoc
        }
    }

    Timer {
        id: connectionTimeout
        interval: 10000
        repeat: false
        onTriggered: {
            modbusClient.disconnectFromInverter()
            root.connectionTestRunning = false
            root.connectionFeedback = "✕ Connection timed out after 10 seconds"
            root.connectionFeedbackColor = "#ff6b78"
        }
    }

    Loader {
        id: dashboardLoader
        anchors.fill: parent
        active: root.dashboardVisible
        source: "DashboardView.qml"
        z: 2
        onLoaded: {
            item.offline = !root.connectionLive
            item.solarPower = telemetry.solarPower
            item.gridPower = telemetry.gridPower
            item.houseLoad = telemetry.houseLoad
            item.batteryPower = telemetry.batteryPower
            item.batterySoc = telemetry.batterySoc
        }
    }

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
            onClicked: {
                root.testConnectionClicked()
                if (root.connectionTestRunning) {
                    connectionTimeout.stop()
                    modbusClient.disconnectFromInverter()
                    root.connectionTestRunning = false
                    root.connectionFeedback = "Connection cancelled"
                    root.connectionFeedbackColor = "#9aa0a8"
                    return
                }
                if (!/^([0-9]{1,3}\.){3}[0-9]{1,3}$/.test(ipField.text)) {
                    root.connectionTestRunning = false
                    root.connectionFeedback = "✕ Invalid IPv4 address"
                    root.connectionFeedbackColor = "#ff6b78"
                } else if (Number(portField.text) < 1 || Number(portField.text) > 65535) {
                    root.connectionTestRunning = false
                    root.connectionFeedback = "✕ Port must be 1–65535"
                    root.connectionFeedbackColor = "#ff6b78"
                } else {
                    root.connectionTestRunning = true
                    root.connectionFeedback = "Checking connection…"
                    root.connectionFeedbackColor = "#f0c674"
                    connectionTimeout.start()
                    modbusClient.connectToInverter(ipField.text, Number(portField.text))
                }
            }
            
            KeyNavigation.up: portField
            KeyNavigation.down: saveButton
            KeyNavigation.tab: saveButton

            contentItem: Row {
                spacing: 14
                anchors.centerIn: parent
                BusyIndicator {
                    running: root.connectionTestRunning
                    visible: running
                    width: 28
                    height: 28
                }
                Text {
                    text: root.connectionTestRunning ? "Cancel connection" : testButton.text
                    color: "#ffffff"
                    font.pixelSize: 24
                    anchors.verticalCenter: parent.verticalCenter
                }
            }
            background: Rectangle {
                color: root.connectionTestRunning ? "#007acc" : (testButton.activeFocus ? "#007acc" : "#252526")
                radius: 8
            }
        }

        Text {
            width: parent.width
            text: root.connectionFeedback
            color: root.connectionFeedbackColor
            font.pixelSize: 22
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        Button {
            id: saveButton
            width: parent.width
            text: "Save & Start"
            onClicked: {
                root.saveAndStartClicked()
                root.dashboardVisible = true
            }

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
