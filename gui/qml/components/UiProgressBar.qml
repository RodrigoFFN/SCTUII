import QtQuick
import QtQuick.Controls

ProgressBar {
    id: control
    required property var design
    property color fillColor: design.accent
    implicitHeight: 5
    background: Rectangle { implicitHeight: 5; color: control.design.background; radius: 2 }
    contentItem: Item {
        Rectangle { width: control.visualPosition*parent.width; height: parent.height; color: control.fillColor; radius: 2 }
    }
}
