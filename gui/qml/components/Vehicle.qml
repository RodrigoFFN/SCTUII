import QtQuick

Item {
    id: car
    required property bool bus
    required property int identifier
    property bool stopped: false
    property color directionColor: "#267e78"
    width: bus ? 28 : 25
    height: bus ? 60 : 44
    readonly property color paint: bus ? ["#d6bd7c","#e1cf98"][identifier%2] : [directionColor,Qt.lighter(directionColor,1.40),"#d7ded8",Qt.lighter(directionColor,1.15)][identifier%4]
    Behavior on opacity { NumberAnimation { duration: 100 } }
    // La parte delantera del dibujo apunta hacia arriba.
    Repeater {
        model: 4
        delegate: Rectangle {
            required property int index
            x: index%2 ? car.width-3 : -1; y: index<2 ? 6 : car.height-12
            width: 4; height: 8; radius: 1; color: "#262d31"
        }
    }
    Rectangle { x: 1; y: 1; width: car.width-1; height: car.height; radius: car.bus?3:6; color: "#333a40"; opacity: 0.45 }
    Rectangle { anchors.fill: parent; anchors.leftMargin: 2; anchors.rightMargin: 2; radius: car.bus?3:6; color: car.paint; border.color: "#444e54"; border.width: 1 }
    Rectangle { x: 4; y: car.bus?5:9; width: car.width-8; height: car.bus?8:7; radius: 2; color: "#344b58"; border.color: "#6a818d"; border.width: 0.7 }
    Rectangle { x: 5; y: car.height-(car.bus?9:12); width: car.width-10; height: 5; radius: 1; color: "#40535d" }
    Rectangle { x: 6; y: car.bus?16:18; width: car.width-12; height: car.bus?27:10; radius: 2; color: car.paint; border.color: "#7f8b91"; border.width: 0.6 }
    Repeater {
        model: car.bus ? 8 : 4
        delegate: Rectangle {
            required property int index
            width: 2.5; height: car.bus?5:4
            x: index%2 ? car.width-5 : 2.5
            y: (car.bus?16:18)+Math.floor(index/2)*(car.bus?7:6)
            radius: 0.8; color: "#344b58"
        }
    }
    Repeater {
        model: 4
        delegate: Rectangle {
            required property int index
            x: index%2 ? car.width-7 : 4; y: index<2 ? 2 : car.height-4
            width: 3; height: 2; radius: 0.6; color: index<2?"#fff7d8":(car.stopped?"#f07860":"#ad473d")
        }
    }
}
