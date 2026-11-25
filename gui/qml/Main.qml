import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import "components"

ApplicationWindow {
    id: window
    objectName: "mainWindow"
    width: 1440; height: 900
    minimumWidth: 1120; minimumHeight: 680
    visible: true
    title: "SimTrrafic — Simulador de intersección"
    color: theme.background
    font.family: Qt.platform.os === "windows" ? "Segoe UI" : "DejaVu Sans"
    font.pixelSize: theme.normal
    Theme { id: theme }
    // Paleta completa: los controles auxiliares y diálogos tampoco heredan fondos oscuros.
    palette.window: theme.panel
    palette.base: theme.panel
    palette.alternateBase: theme.surface
    palette.button: theme.surface
    palette.light: theme.panel
    palette.midlight: theme.surface
    palette.dark: theme.muted
    palette.mid: theme.border
    palette.text: theme.text
    palette.windowText: theme.text
    palette.buttonText: theme.text
    palette.highlight: theme.accent
    palette.highlightedText: "white"
    palette.placeholderText: theme.muted
    FileDialog {
        id: exportDialog; objectName: "exportDialog"
        title: "Exportar bitácora de simulación"
        fileMode: FileDialog.SaveFile
        nameFilters: ["Archivos CSV (*.csv)"]
        defaultSuffix: "csv"
        onAccepted: simulation.exportCsv(selectedFile)
    }
    Shortcut { sequence: "Ctrl+Return"; enabled: !simulation.status.running; onActivated: controls.start() }
    Shortcut { sequence: "Ctrl+E"; enabled: simulation.eventCount>0; onActivated: exportDialog.open() }
    ColumnLayout {
        anchors.fill: parent; spacing: 0
        TopBar { Layout.fillWidth: true; Layout.preferredHeight: 56; design: theme }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            ControlPanel {
                id: controls; objectName: "controlPanel"
                Layout.preferredWidth: 272; Layout.fillHeight: true; design: theme
            }
            Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: theme.border }
            SplitView {
                id: workspace
                Layout.fillWidth: true; Layout.fillHeight: true; Layout.margins: 12
                orientation: Qt.Vertical
                handle: Rectangle {
                    implicitHeight: 10; color: theme.background
                    Rectangle { anchors.centerIn: parent; width: 32; height: 2; radius: 1; color: SplitHandle.hovered ? theme.accent : theme.strongBorder }
                }
                TrafficScene {
                    objectName: "trafficScene"
                    SplitView.fillHeight: true; SplitView.minimumHeight: 330
                    design: theme; reducedMotion: controls.reducedMotion
                }
                EventLog {
                    id: eventLog; objectName: "eventLog"
                    SplitView.preferredHeight: collapsed ? 40 : Math.max(150,Math.min(184,window.height*0.22))
                    SplitView.minimumHeight: collapsed ? 40 : 120
                    SplitView.maximumHeight: collapsed ? 40 : window.height*0.45
                    design: theme; onExportRequested: exportDialog.open()
                }
            }
            Rectangle { Layout.preferredWidth: 1; Layout.fillHeight: true; color: theme.border }
            StatisticsPanel {
                objectName: "statisticsPanel"
                Layout.preferredWidth: window.width < 1350 ? 226 : 248
                Layout.fillHeight: true; design: theme; plannedVehicles: controls.expected
            }
        }
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 28
            color: theme.panel
            Rectangle { width: parent.width; height: 1; color: theme.border }
            RowLayout {
                anchors.fill: parent; anchors.leftMargin: 16; anchors.rightMargin: 16; spacing: 8
                Label {
                    Layout.fillWidth: true; elide: Text.ElideRight
                    text: simulation.message.length ? simulation.message : "Vista ilustrativa · sentidos N → S y O → E · posiciones sin escala física"
                    color: simulation.messageIsError ? theme.red : simulation.message.length ? theme.accent : theme.muted
                    font.pixelSize: 10
                }
                UiButton { design: theme; visible: simulation.message.length > 0; text: "Cerrar"; implicitHeight: 22; onClicked: simulation.clearMessage() }
                Label { text: "SCTUII  /  Qt Quick"; color: theme.muted; font.pixelSize: 10 }
            }
        }
    }
}
