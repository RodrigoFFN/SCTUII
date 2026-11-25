import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root
    required property var design
    property bool reducedMotion: false
    color: design.panel; border.color: design.border; radius: 4
    onReducedMotionChanged: simulation.setReducedMotion(reducedMotion)
    ColumnLayout {
        anchors.fill: parent; anchors.margins: 1; spacing: 0
        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 44; Layout.leftMargin: 14; Layout.rightMargin: 14; spacing: 10
            Label { text: "Intersección"; font.pixelSize: 15; font.weight: Font.DemiBold; color: root.design.text }
            Label { text: "01 / DOS EJES"; font.pixelSize: 9; font.letterSpacing: 1; color: root.design.muted; visible: root.width>620 }
            Item { Layout.fillWidth: true }
            UiCheckBox { id: zone; design: root.design; text: "Zona crítica"; checked: false; Accessible.name: "Mostrar límite de la zona crítica" }
        }
        Rectangle {
            id: viewport; objectName: "mapViewport"
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            color: "#bac6b3"
            readonly property real factor: Math.max(0.1, Math.min(width/740, height/500))
            function syncExtent() { simulation.setSceneExtent(width/(2*factor),height/(2*factor)) }
            onWidthChanged: syncExtent(); onHeightChanged: syncExtent(); Component.onCompleted: syncExtent()
            Item {
                id: world
                width: viewport.width/viewport.factor; height: viewport.height/viewport.factor
                anchors.centerIn: parent; scale: viewport.factor; transformOrigin: Item.Center
                readonly property real cx: width/2
                readonly property real cy: height/2
                readonly property real roadHalf: 70
                Rectangle { x: world.cx-70; width: 140; height: world.height; color: root.design.road }
                Rectangle { y: world.cy-70; width: world.width; height: 140; color: root.design.road }
                Repeater {
                    model: 4
                    UrbanBlock {
                        required property int index
                        kind: index
                        x: index%2 ? world.cx+world.roadHalf+3 : -8
                        y: index>=2 ? world.cy+world.roadHalf+3 : -8
                        width: world.cx-world.roadHalf+5
                        height: world.cy-world.roadHalf+5
                    }
                }
                Repeater {
                    model: 4
                    Rectangle {
                        required property int index
                        x: world.cx+(index%2?66:-68)
                        y: index<2?0:world.cy+83
                        width: 2; height: Math.max(0,world.cy-83); color: "#bdc5be"
                    }
                }
                Repeater {
                    model: 4
                    Rectangle {
                        required property int index
                        x: index<2?0:world.cx+83
                        y: world.cy+(index%2?66:-68)
                        width: Math.max(0,world.cx-83); height: 2; color: "#bdc5be"
                    }
                }
                Repeater {
                    model: Math.ceil(world.height/32)
                    Rectangle {
                        required property int index
                        x: world.cx-1; y: index*32; width: 2; height: 16; color: "#d8c892"
                        visible: y+height<world.cy-85 || y>world.cy+85
                    }
                }
                Repeater {
                    model: Math.ceil(world.width/32)
                    Rectangle {
                        required property int index
                        x: index*32; y: world.cy-1; width: 16; height: 2; color: "#d8c892"
                        visible: x+width<world.cx-85 || x>world.cx+85
                    }
                }
                // Marcas transversales y líneas de detención. No hay agentes peatonales.
                Repeater {
                    model: 8
                    Item {
                        required property int index
                        Rectangle { x: world.cx-61+index*16; y: world.cy-82; width: 9; height: 12; color: "#d3d9ce" }
                        Rectangle { x: world.cx-61+index*16; y: world.cy+70; width: 9; height: 12; color: "#d3d9ce" }
                        Rectangle { x: world.cx-82; y: world.cy-61+index*16; width: 12; height: 9; color: "#d3d9ce" }
                        Rectangle { x: world.cx+70; y: world.cy-61+index*16; width: 12; height: 9; color: "#d3d9ce" }
                    }
                }
                Rectangle { x: world.cx-67; y: world.cy-94; width: 65; height: 3; color: "#f0f1df" }
                Rectangle { x: world.cx-94; y: world.cy+2; width: 3; height: 65; color: "#f0f1df" }
                Rectangle {
                    x: world.cx-68; y: world.cy-68; width: 136; height: 136
                    color: simulation.status.occupied ? "#1877b29e" : "#0cffffff"
                    border.width: 1; border.color: simulation.status.occupied ? "#94bdb0" : "#a1b4af"; visible: zone.checked
                }
                component RoadArrow: Item {
                    width: 22; height: 30
                    Rectangle { x: 10; y: 5; width: 3; height: 24; color: "#bdc8c3" }
                    Rectangle { x: 4; y: 5; width: 11; height: 3; rotation: -45; color: "#bdc8c3" }
                    Rectangle { x: 10; y: 5; width: 11; height: 3; rotation: 45; color: "#bdc8c3" }
                }
                RoadArrow { x: world.cx-46; y: world.cy-164; rotation: 180 }
                RoadArrow { x: world.cx-174; y: world.cy+19; rotation: 90 }
                RoadArrow { x: world.cx-46; y: world.cy+154; rotation: 180 }
                RoadArrow { x: world.cx+162; y: world.cy+19; rotation: 90 }
                Label { x: world.cx+23; y: 15; text: "N"; color: "#e1e8e3"; font.pixelSize: 13; font.weight: Font.DemiBold }
                Label { x: world.cx+23; y: world.height-32; text: "S"; color: "#e1e8e3"; font.pixelSize: 13; font.weight: Font.DemiBold }
                Label { x: 16; y: world.cy-45; text: "O"; color: "#e1e8e3"; font.pixelSize: 13; font.weight: Font.DemiBold }
                Label { x: world.width-28; y: world.cy-45; text: "E"; color: "#e1e8e3"; font.pixelSize: 13; font.weight: Font.DemiBold }
                TrafficLight { x: world.cx-103; y: world.cy-132; green: simulation.status.phase==="ns"; design: root.design; z: 3 }
                TrafficLight { x: world.cx-126; y: world.cy+87; rotation: -90; green: simulation.status.phase==="eo"; design: root.design; z: 3 }
                Repeater {
                    model: simulation.vehicles
                    Vehicle {
                        required property int vehicleId
                        required property string axis
                        required property bool isBus
                        required property real travel
                        required property bool onMap
                        required property int vehicleStage
                        identifier: vehicleId; bus: isBus; visible: onMap
                        stopped: vehicleStage===1
                        directionColor: axis==="ns" ? root.design.ns : root.design.eo
                        opacity: visible ? 1 : 0
                        x: world.cx+(axis==="ns"?-35:travel)-width/2
                        y: world.cy+(axis==="ns"?travel:35)-height/2
                        rotation: axis==="ns"?180:90
                        z: 2
                    }
                }
            }
            Rectangle {
                anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 12
                width: queueText.implicitWidth+20; height: 27; radius: 3; color: "#f4f7f0"; border.color: "#b7c5b4"
                visible: simulation.hiddenNS+simulation.hiddenEO>0
                Label { id: queueText; anchors.centerIn: parent; text: "Cola fuera de vista   N–S +"+simulation.hiddenNS+"   E–O +"+simulation.hiddenEO; font.pixelSize: 10; color: root.design.text }
            }
            Rectangle {
                anchors.bottom: parent.bottom; anchors.right: parent.right; anchors.margins: 10
                width: 104; height: 25; radius: 3; color: "#eaf0e6"; border.color: "#b5c2b0"
                Label { anchors.centerIn: parent; text: "VISTA CENITAL"; font.pixelSize: 9; font.letterSpacing: 1; color: "#526855" }
            }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 40; Layout.leftMargin: 14; Layout.rightMargin: 14; spacing: 7
            Rectangle { Layout.preferredWidth: 7; Layout.preferredHeight: 7; radius: 4; color: simulation.status.phase==="ns"?root.design.green:root.design.red }
            Label { text: "N–S"; color: root.design.text; font.pixelSize: 11 }
            Rectangle { Layout.preferredWidth: 7; Layout.preferredHeight: 7; Layout.leftMargin: 9; radius: 4; color: simulation.status.phase==="eo"?root.design.green:root.design.red }
            Label { text: "E–O"; color: root.design.text; font.pixelSize: 11 }
            Rectangle { Layout.preferredWidth: 1; Layout.preferredHeight: 13; Layout.leftMargin: 6; Layout.rightMargin: 6; color: root.design.border }
            Label { text: simulation.status.phase==="ns"?"Paso Norte–Sur":simulation.status.phase==="eo"?"Paso Este–Oeste":"Ambos en rojo"; font.pixelSize: 11; color: root.design.text; Layout.fillWidth: true; elide: Text.ElideRight }
            Label { text: simulation.status.occupied ? "Cruce ocupado" : "Cruce libre"; font.pixelSize: 11; color: simulation.status.occupied ? root.design.accent : root.design.muted }
            Rectangle { Layout.preferredWidth: 6; Layout.preferredHeight: 6; radius: 3; color: simulation.status.occupied ? root.design.accent : root.design.strongBorder }
        }
    }
}
