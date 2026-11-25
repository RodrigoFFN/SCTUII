import QtQuick
import QtQuick.Controls

CheckBox {
    id: control
    required property var design
    implicitHeight: 28; spacing: 7; padding: 0; hoverEnabled: true
    indicator: Rectangle {
        implicitWidth: 16; implicitHeight: 16
        x: control.leftPadding; y: (control.height-height)/2; radius: 3
        color: control.checked ? control.design.accent : control.design.panel
        border.color: control.activeFocus ? control.design.accent : control.design.strongBorder
        border.width: control.activeFocus ? 2 : 1
        Item {
            anchors.centerIn: parent; width: 10; height: 9; visible: control.checked
            Rectangle { x: 0; y: 4; width: 5; height: 2; rotation: 45; color: "white" }
            Rectangle { x: 3; y: 3; width: 8; height: 2; rotation: -48; color: "white" }
        }
    }
    contentItem: Text {
        text: control.text; font.pixelSize: 11
        leftPadding: control.indicator.width+control.spacing
        color: control.enabled ? control.design.muted : control.design.disabledText
        verticalAlignment: Text.AlignVCenter
    }
}
