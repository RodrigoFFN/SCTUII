import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property var design
    property int plannedVehicles: 12
    readonly property int expected: simulation.status.running || simulation.status.finished ? simulation.status.expected : plannedVehicles
    color: design.panel
    component Metric: RowLayout {
        id: metric
        required property string label
        required property string value
        Layout.fillWidth: true; spacing: 5
        Label { text: metric.label; color: root.design.muted; Layout.fillWidth: true; font.pixelSize: 12 }
        Label { text: metric.value; color: root.design.text; font.pixelSize: 20; font.weight: Font.DemiBold }
    }
    component AxisStats: ColumnLayout {
        id: axisItem
        required property string name
        required property int generated
        required property int served
        required property real wait
        required property color axisColor
        Layout.fillWidth: true; spacing: 5
        RowLayout {
            Layout.fillWidth: true
            Rectangle { Layout.preferredWidth: 3; Layout.preferredHeight: 13; color: axisItem.axisColor }
            Label { text: axisItem.name; color: root.design.text; font.weight: Font.Medium; Layout.fillWidth: true }
            Label { text: axisItem.served+" / "+axisItem.generated; color: root.design.text; font.pixelSize: 12 }
        }
        UiProgressBar { design: root.design; fillColor: axisItem.axisColor; Layout.fillWidth: true; from: 0; to: Math.max(1,axisItem.generated); value: axisItem.served }
        Label { text: "Espera media: "+Math.round(axisItem.wait)+" ms"; font.pixelSize: 11; color: root.design.muted }
    }
    ScrollView {
        id: scroll
        anchors.fill: parent; anchors.margins: 16; contentWidth: availableWidth; clip: true
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ColumnLayout {
            width: scroll.availableWidth; spacing: 12
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Resultados"; font.pixelSize: 16; font.weight: Font.DemiBold; color: root.design.text; Layout.fillWidth: true }
                Label { text: "02"; color: root.design.muted; font.pixelSize: 11 }
            }
            Label { text: "VEHÍCULOS ATENDIDOS"; font.pixelSize: 10; font.letterSpacing: 1; color: root.design.muted; Layout.topMargin: 6 }
            RowLayout {
                Layout.fillWidth: true; spacing: 6
                Label { text: simulation.status.served; color: root.design.text; font.pixelSize: 40; font.weight: Font.Medium }
                Label { text: "/ "+root.expected; color: root.design.muted; font.pixelSize: 18; Layout.alignment: Qt.AlignBottom; Layout.bottomMargin: 7 }
                Item { Layout.fillWidth: true }
                Label { text: Math.round(100*simulation.status.served/Math.max(1,root.expected))+" %"; color: root.design.accent; font.pixelSize: 12 }
            }
            UiProgressBar { design: root.design; Layout.fillWidth: true; from: 0; to: Math.max(1,root.expected); value: simulation.status.served }
            Metric { label: "Generados"; value: String(simulation.status.generated) }
            Metric { label: "Pendientes"; value: String(simulation.status.pending) }
            Metric { label: "Espera media"; value: Math.round(simulation.status.wait)+" ms" }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.design.border; Layout.topMargin: 4; Layout.bottomMargin: 4 }
            Label { text: "Rendimiento por eje"; font.weight: Font.Medium; color: root.design.text }
            AxisStats { name: "Norte–Sur"; axisColor: root.design.ns; generated: simulation.status.nsGenerated; served: simulation.status.nsServed; wait: simulation.status.nsWait }
            AxisStats { name: "Este–Oeste"; axisColor: root.design.eo; generated: simulation.status.eoGenerated; served: simulation.status.eoServed; wait: simulation.status.eoWait }
            Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: root.design.border }
            Label { text: "Evolución de atención"; font.weight: Font.Medium; color: root.design.text }
            Canvas {
                id: graph; Layout.fillWidth: true; Layout.preferredHeight: Math.max(100,Math.min(144,root.height-510))
                onWidthChanged: requestPaint(); onHeightChanged: requestPaint()
                Connections { target: simulation; function onHistoryChanged() { graph.requestPaint() } }
                onPaint: {
                    var ctx=getContext("2d"); ctx.clearRect(0,0,width,height)
                    var points=simulation.history, maxT=points.length?Math.max(1000,points[points.length-1].time):1000
                    var maxY=Math.max(4, Math.ceil((points.length?points[points.length-1].total:0)/4)*4)
                    var l=25,r=width-8,t=9,b=height-21
                    ctx.font="10px sans-serif"; ctx.fillStyle=String(root.design.muted); ctx.strokeStyle=String(root.design.border); ctx.lineWidth=1
                    for(var i=0;i<=4;i++){var y=b-(b-t)*i/4;ctx.beginPath();ctx.moveTo(l,y);ctx.lineTo(r,y);ctx.stroke();ctx.fillText(String(maxY*i/4),1,y+3)}
                    ctx.fillText("0 s",l,b+15); ctx.fillText((maxT/1000).toFixed(1)+" s",Math.max(l,r-30),b+15)
                    if(!points.length)return
                    ctx.beginPath();ctx.moveTo(l,b);ctx.strokeStyle=String(root.design.accent);ctx.lineWidth=2
                    for(var j=0;j<points.length;j++){var x=l+(r-l)*points[j].time/maxT;var yy=b-(b-t)*points[j].total/maxY;ctx.lineTo(x,j===0?b:b-(b-t)*points[j-1].total/maxY);ctx.lineTo(x,yy)}ctx.stroke()
                }
            }
            Label { text: "Acumulado de pasos reales"; color: root.design.muted; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
        }
    }
}
