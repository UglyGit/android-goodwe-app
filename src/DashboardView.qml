import QtQuick

Item {
    id: root
    property real solarPower: 0
    property real gridPower: 0
    property real houseLoad: 0
    property real batteryPower: 0
    property int batterySoc: -1
    property bool offline: true
    readonly property color textColor: offline ? "#737984" : "#f3f3f3"
    readonly property color inactiveColor: "#62666d"

    Rectangle { anchors.fill: parent; color: "#191a1c" }
    Text { x: 48; y: 34; text: "Energy distribution"; color: root.textColor; font.pixelSize: 30; font.weight: Font.Light }

    Item {
        id: panel
        width: Math.min(parent.width * .90, 520)
        height: Math.min(parent.height * .88, 900)
        anchors.centerIn: parent
        property real d: Math.min(width * .25, 150)
        property real cx: width / 2
        property real cy: height * .50

        Canvas { anchors.fill: parent; onPaint: {
            var c = getContext("2d"); c.reset(); c.lineWidth = 2
            var x = panel.cx, y = height * .38, r = panel.d / 2
            c.strokeStyle = root.offline ? root.inactiveColor : "#725ca0"; c.beginPath(); c.moveTo(x, height * .10 + panel.d); c.lineTo(x, height * .72); c.stroke()
            c.strokeStyle = root.offline ? root.inactiveColor : "#168fca"; c.beginPath(); c.moveTo(width * .12 + r, y); c.lineTo(x - r, y); c.stroke()
            c.strokeStyle = root.offline ? root.inactiveColor : "#f0a500"; c.beginPath(); c.moveTo(x + r, y); c.lineTo(width * .88 - r, y); c.stroke()
            c.fillStyle = root.offline ? root.inactiveColor : "#725ca0"; c.beginPath(); c.arc(x, height * .30, 4, 0, Math.PI * 2); c.fill()
            c.fillStyle = root.offline ? root.inactiveColor : "#168fca"; c.beginPath(); c.arc(x - width * .18, y, 4, 0, Math.PI * 2); c.fill()
            c.fillStyle = root.offline ? root.inactiveColor : "#58bba9"; c.beginPath(); c.arc(x + width * .18, y, 4, 0, Math.PI * 2); c.fill()
            c.fillStyle = root.offline ? root.inactiveColor : "#f0498e"; c.beginPath(); c.arc(x, height * .55, 4, 0, Math.PI * 2); c.fill()
        } }

        Loader { width: panel.d; height: panel.d + 38; x: panel.cx - panel.d / 2; y: panel.height * .10; sourceComponent: nodeComponent; onLoaded: { item.accent=root.offline ? root.inactiveColor : "#f0a500"; item.glyph="☀"; item.title="Solar"; item.value=root.solarPower.toFixed(1)+" kW" } }
        Loader { width: panel.d; height: panel.d + 38; x: panel.width * .12; y: panel.height * .38 - panel.d / 2; sourceComponent: nodeComponent; onLoaded: { item.accent=root.offline ? root.inactiveColor : "#168fca"; item.glyph="⌁"; item.title="Grid"; item.value=(root.gridPower < 0 ? "↔ " : "→ ")+Math.abs(root.gridPower).toFixed(1)+" kW" } }
        Loader { width: panel.d; height: panel.d + 38; x: panel.width * .88 - panel.d; y: panel.height * .38 - panel.d / 2; sourceComponent: nodeComponent; onLoaded: { item.accent=root.offline ? root.inactiveColor : "#f0a500"; item.glyph="⌂"; item.title="Home"; item.value=root.houseLoad.toFixed(1)+" kW" } }
        Loader { width: panel.d; height: panel.d + 38; x: panel.cx - panel.d / 2; y: panel.height * .72; sourceComponent: nodeComponent; onLoaded: { item.accent=root.offline ? root.inactiveColor : "#f0498e"; item.glyph="▣"; item.title="Battery"; item.value=(root.batteryPower < 0 ? "↓ " : "↑ ")+Math.abs(root.batteryPower).toFixed(1)+" kW"; item.subvalue=root.batterySoc < 0 ? "SOC --" : "SOC "+root.batterySoc+"%" } }

        Component { id: nodeComponent
            Item {
                property color accent: "white"
                property string glyph: ""
                property string title: ""
                property string value: ""
                property string subvalue: ""
                width: panel.d; height: panel.d + 38
                Rectangle { width: panel.d; height: panel.d; radius: panel.d / 2; color: "#191a1c"; border.width: accent === "#f0a500" && title === "Home" ? 5 : 2; border.color: accent }
                Text { anchors.horizontalCenter: parent.horizontalCenter; y: panel.d * .27; text: glyph; color: root.textColor; font.pixelSize: panel.d * .22 }
                Text { anchors.horizontalCenter: parent.horizontalCenter; y: panel.d * .58; text: value; color: accent; font.pixelSize: Math.max(12, panel.d * .11); font.bold: true }
                Text { anchors.top: parent.bottom; anchors.topMargin: 5; anchors.horizontalCenter: parent.horizontalCenter; text: title; color: root.textColor; font.pixelSize: 14 }
                Text { anchors.top: parent.bottom; anchors.topMargin: 24; anchors.horizontalCenter: parent.horizontalCenter; text: subvalue; color: accent; font.pixelSize: 12 }
            }
        }
    }
}
