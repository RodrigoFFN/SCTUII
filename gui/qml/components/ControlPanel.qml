import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property var design
    property alias reducedMotion: reduce.checked
    readonly property int expected: count.value*2
    color: design.panel
    function preset(i) {
        var values = [[3,650,750,2500,350,0],[6,350,650,2400,350,0],[16,130,550,2000,300,1]]
        var v=values[i]; if (!v) return
        count.value=v[0]; arrival.value=v[1]; crossing.value=v[2]; green.value=v[3]; red.value=v[4]; policy.currentIndex=v[5]
    }
    function start() {
        startButton.forceActiveFocus()
        simulation.start(count.value,arrival.value,crossing.value,green.value,red.value,policy.currentIndex===1)
    }
    component Section: Label {
        color: root.design.muted; font.pixelSize: 10; font.weight: Font.DemiBold; font.letterSpacing: 1.2
        Layout.fillWidth: true; Layout.topMargin: 12; Layout.bottomMargin: 2
    }
    component FieldLabel: Label { color: root.design.text; font.pixelSize: 12; Layout.fillWidth: true }
    component NumberField: UiSpinBox {
        design: root.design; Layout.preferredWidth: 126; Layout.preferredHeight: 34
        onValueModified: scenario.currentIndex=3
    }
    ScrollView {
        id: scroll
        anchors.fill: parent; anchors.margins: 16
        contentWidth: availableWidth; clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ColumnLayout {
            width: scroll.availableWidth; spacing: 8
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Configuración"; color: root.design.text; font.pixelSize: 16; font.weight: Font.DemiBold; Layout.fillWidth: true }
                Label { text: "01"; color: root.design.muted; font.pixelSize: 11 }
            }
            Section { text: "ESCENARIO"; Layout.topMargin: 10 }
            UiComboBox {
                id: scenario; objectName: "scenarioBox"; design: root.design
                Layout.fillWidth: true
                model: ["Flujo bajo", "Flujo normal", "Hora punta", "Personalizado"]
                currentIndex: 1; enabled: !simulation.status.running
                onActivated: function(index) { root.preset(index) }
                Accessible.name: "Escenario"
            }
            Section { text: "DEMANDA Y CRUCE" }
            GridLayout {
                Layout.fillWidth: true; columns: 2; columnSpacing: 6; rowSpacing: 9
                enabled: !simulation.status.running
                FieldLabel { text: "Vehículos / eje" }
                NumberField { id: count; objectName: "countField"; from: 1; to: 60; value: 6; Accessible.name: "Vehículos por eje" }
                FieldLabel { text: "Llegada · ms" }
                NumberField { id: arrival; objectName: "arrivalField"; from: 10; to: 5000; stepSize: 10; value: 350; Accessible.name: "Intervalo de llegada en milisegundos" }
                FieldLabel { text: "Cruce · ms" }
                NumberField { id: crossing; objectName: "crossingField"; from: 20; to: 5000; stepSize: 10; value: 650; Accessible.name: "Tiempo de cruce en milisegundos" }
            }
            Section { text: "SEMÁFOROS" }
            UiComboBox {
                id: policy; objectName: "policyBox"; design: root.design; Layout.fillWidth: true
                model: ["Ciclo fijo", "Adaptativa"]
                enabled: !simulation.status.running
                onActivated: scenario.currentIndex=3
                Accessible.name: "Política semafórica"
            }
            GridLayout {
                Layout.fillWidth: true; columns: 2; columnSpacing: 6; rowSpacing: 9
                enabled: !simulation.status.running
                FieldLabel { text: "Verde · ms" }
                NumberField { id: green; objectName: "greenField"; from: 100; to: 10000; stepSize: 50; value: 2400; Accessible.name: "Duración del verde en milisegundos" }
                FieldLabel { text: "Todo rojo · ms" }
                NumberField { id: red; objectName: "redField"; from: 20; to: 5000; stepSize: 10; value: 350; Accessible.name: "Transición en rojo en milisegundos" }
            }
            Label {
                text: policy.currentIndex===1 ? "El verde se ajusta según la demanda de cada eje." : "Los ejes alternan con una duración de verde constante."
                Layout.fillWidth: true; Layout.topMargin: 2; wrapMode: Text.WordWrap
                font.pixelSize: 11; color: root.design.muted
            }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; Layout.topMargin: 10; Layout.bottomMargin: 5; color: root.design.border }
            UiButton {
                id: startButton; objectName: "startButton"; design: root.design; primary: true
                Layout.fillWidth: true; Layout.preferredHeight: 38
                symbol: "▷"; text: "Iniciar simulación"; enabled: !simulation.status.running
                onClicked: root.start()
                ToolTip.visible: enabled && hovered; ToolTip.text: "Iniciar · Ctrl+Return"
            }
            RowLayout {
                Layout.fillWidth: true; spacing: 8
                UiButton { objectName: "stopButton"; design: root.design; text: "Detener"; symbol: "□"; Layout.fillWidth: true; enabled: simulation.status.running && !simulation.stopping; onClicked: simulation.stop() }
                UiButton { objectName: "resetButton"; design: root.design; text: "Reiniciar"; symbol: "↺"; Layout.fillWidth: true; enabled: !simulation.status.running; onClicked: simulation.reset() }
            }
            UiCheckBox { id: reduce; objectName: "reduceMotion"; design: root.design; text: "Reducir movimiento"; Layout.topMargin: 8 }

        }
    }
}
