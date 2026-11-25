import QtQuick
import QtQuick.Controls

Button {
    id: control
    required property var design
    property bool primary: false
    property string symbol: ""
    implicitHeight: design.controlHeight
    implicitWidth: Math.max(80, label.implicitWidth + 24)
    padding: 10
    hoverEnabled: true
    font.pixelSize: 12; font.weight: Font.Medium
    contentItem: Text {
        id: label
        text: (control.symbol.length ? control.symbol + "  " : "") + control.text
        font: control.font
        color: !control.enabled ? control.design.disabledText : control.primary ? "white" : control.design.text
        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: control.design.radius
        color: !control.enabled ? control.design.surface : control.primary ? (control.down || control.hovered ? control.design.accentHover : control.design.accent) : control.down ? control.design.accentLight : control.hovered ? control.design.surface : control.design.panel
        border.color: control.activeFocus ? control.design.accent : control.primary && control.enabled ? control.design.accent : control.design.strongBorder
        border.width: control.activeFocus ? 2 : 1
        Behavior on color { ColorAnimation { duration: 100 } }
    }
}
