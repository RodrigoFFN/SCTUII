import QtQuick

Item {
    id: root
    required property bool green
    required property var design
    width: 24; height: 46
    Rectangle { x: 10; y: 25; width: 4; height: 21; color: "#87948c" }
    Rectangle { x: 2; y: 2; width: 23; height: 33; radius: 4; color: "#31453d"; opacity: 0.18 }
    Rectangle {
        width: 22; height: 33; radius: 4; color: "#263435"; border.color: "#1b282c"
        Rectangle { x: 5; y: 4; width: 12; height: 11; radius: 6; color: root.green?"#583f38":"#ef7764"; border.color: "#1e2d2e" }
        Rectangle { x: 5; y: 18; width: 12; height: 11; radius: 6; color: root.green?"#8ed6a0":"#42564b"; border.color: "#1e2d2e" }
        Rectangle { x: 8; y: root.green?20:6; width: 4; height: 3; radius: 2; color: root.green?"#c7edd1":"#ffb0a0" }
    }
}
