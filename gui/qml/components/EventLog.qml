import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property var design
    signal exportRequested()
    property bool collapsed: false
    color: design.panel; border.color: design.border; radius: 4
    component Cell: Label { font.pixelSize: 11; elide: Text.ElideRight; color: root.design.muted; verticalAlignment: Text.AlignVCenter }
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 1; spacing: 0
        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 38; Layout.leftMargin: 10; Layout.rightMargin: 8; spacing: 8
            Label { text: "Bitácora"; font.pixelSize: 14; font.weight: Font.DemiBold; color: root.design.text }
            Label { text: simulation.eventCount+" eventos"; font.pixelSize: 11; color: root.design.muted }
            Item { Layout.fillWidth: true }
            UiCheckBox { design: root.design; id: follow; text: "Seguir"; checked: true; font.pixelSize: 11; onToggled: if (checked) log.positionViewAtBeginning() }
            UiButton { objectName: "exportButton"; design: root.design; text: "Exportar CSV"; Layout.preferredHeight: 27; enabled: simulation.eventCount>0; onClicked: root.exportRequested() }
            UiButton { objectName: "collapseLog"; design: root.design; text: root.collapsed ? "+" : "−"; implicitWidth: 28; Layout.preferredWidth: 28; Layout.preferredHeight: 27; Accessible.name: root.collapsed ? "Expandir bitácora" : "Plegar bitácora"; onClicked: root.collapsed=!root.collapsed }
        }
        Rectangle {
            visible: !root.collapsed; Layout.fillWidth: true; Layout.preferredHeight: 25; color: root.design.surface
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 10; spacing: 8
                Cell { text: "Tiempo (ms)"; Layout.preferredWidth: 75 }
                Cell { text: "Evento"; Layout.preferredWidth: 64 }
                Cell { text: "Vehículo"; Layout.preferredWidth: 55 }
                Cell { text: "Eje"; Layout.preferredWidth: 36 }
                Cell { text: "Descripción"; Layout.fillWidth: true }
            }
        }
        ListView {
            id: log; objectName: "eventList"
            visible: !root.collapsed; focus: true; keyNavigationEnabled: true
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            model: simulation.events; reuseItems: true
            onCountChanged: if (follow && follow.checked) positionViewAtBeginning()
            onMovementStarted: follow.checked=false
            boundsBehavior: Flickable.StopAtBounds
            ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }
            delegate: Rectangle {
                required property int eventTime
                required property string eventType
                required property int eventId
                required property string eventAxis
                required property string eventDescription
                required property int index
                width: log.width; height: 27; color: ListView.isCurrentItem && log.activeFocus ? root.design.accentLight : index%2===0 ? root.design.panel : "#f6f8f8"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 10; anchors.rightMargin: 12; spacing: 8
                    Cell { text: String(eventTime); font.family: Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono"; Layout.preferredWidth: 75 }
                    Cell { text: eventType; color: eventType==="paso"?root.design.green:eventType==="detenido"?root.design.red:root.design.accent; Layout.preferredWidth: 64 }
                    Cell { text: eventId?"#"+eventId:"—"; Layout.preferredWidth: 55 }
                    Cell { text: eventAxis; Layout.preferredWidth: 36 }
                    Cell { text: eventDescription; color: root.design.text; Layout.fillWidth: true; ToolTip.visible: hover.hovered && truncated; ToolTip.text: text; HoverHandler { id: hover } }
                }
            }
            Label { anchors.centerIn: parent; visible: log.count===0; text: "Inicia una simulación para registrar eventos."; color: root.design.muted; font.pixelSize: 12 }
        }
    }
}
