import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property var design
    color: design.header
    function time(ms) {
        var sec=Math.floor(ms/1000)
        return (Math.floor(sec/60)<10?"0":"")+Math.floor(sec/60)+":"+(sec%60<10?"0":"")+(sec%60)+"."+Math.floor(ms%1000/100)
    }
    RowLayout {
        anchors.fill: parent; anchors.leftMargin: 18; anchors.rightMargin: 20; spacing: 12
        Item {
            Layout.preferredWidth: 26; Layout.preferredHeight: 28
            Rectangle { x: 8; width: 10; height: 28; radius: 1; color: "#e0eeea" }
            Rectangle { y: 9; width: 26; height: 10; radius: 1; color: "#e0eeea" }
            Rectangle { x: 12; y: 1; width: 2; height: 6; color: root.design.header }
            Rectangle { x: 12; y: 21; width: 2; height: 6; color: root.design.header }
            Rectangle { x: 1; y: 13; width: 6; height: 2; color: root.design.header }
            Rectangle { x: 20; y: 13; width: 5; height: 2; color: root.design.header }
        }
        Label { text: "SimTrrafic"; font.pixelSize: 19; font.weight: Font.DemiBold; color: "#f5f8f7" }
        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 20; Layout.leftMargin: 6; Layout.rightMargin: 6; color: "#4a5d61" }
        Label { text: "Simulador de intersección"; color: "#b9cacc"; font.pixelSize: 12 }
        Item { Layout.fillWidth: true }
        Rectangle {
            Layout.preferredWidth: 7; Layout.preferredHeight: 7; radius: 4
            color: simulation.status.error.length ? "#ee9687" : simulation.status.running ? "#85c6a4" : "#b9cacc"
        }
        Label {
            text: simulation.stopping ? "Deteniendo…" : simulation.status.error.length ? "Error" : simulation.status.running ? "En ejecución" : simulation.status.finished ? (simulation.status.stopped ? "Detenida" : "Finalizada") : "Lista para iniciar"
            color: "#edf3f1"; font.pixelSize: 12
        }
        Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 20; Layout.leftMargin: 8; Layout.rightMargin: 8; color: "#4a5d61" }
        Label { text: "TIEMPO"; color: "#b9cacc"; font.pixelSize: 9; font.letterSpacing: 1 }
        Label { text: root.time(simulation.status.elapsed); font.family: Qt.platform.os === "windows" ? "Consolas" : "DejaVu Sans Mono"; font.pixelSize: 18; Layout.preferredWidth: 96; color: "#edf3f1" }
    }
}
