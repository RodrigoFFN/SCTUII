import QtQuick
import QtQuick.Controls

SpinBox {
    id: control
    required property var design
    implicitWidth: 126; implicitHeight: design.controlHeight
    editable: true; wheelEnabled: activeFocus; hoverEnabled: true
    leftPadding: 26; rightPadding: 26
    font.pixelSize: 13
    // Sin agrupadores: 10000 debe caber íntegro incluso con DPI alto.
    textFromValue: function(value, locale) { return String(value) }
    valueFromText: function(text, locale) { return Number(text) }
    validator: IntValidator { bottom: Math.min(control.from,control.to); top: Math.max(control.from,control.to) }
    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: control.enabled ? control.design.text : control.design.disabledText
        selectionColor: control.design.accent; selectedTextColor: "white"
        horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter
        readOnly: !control.editable; validator: control.validator
        inputMethodHints: Qt.ImhDigitsOnly
        selectByMouse: true
        Accessible.name: control.Accessible.name
    }
    background: Rectangle {
        color: control.enabled ? control.design.panel : control.design.surface
        border.color: control.activeFocus ? control.design.accent : control.design.strongBorder
        border.width: control.activeFocus ? 2 : 1; radius: control.design.radius
    }
    down.indicator: Rectangle {
        x: 1; y: 1; width: 24; height: control.height-2
        radius: 3; color: control.down.pressed ? control.design.accentLight : control.down.hovered ? control.design.surface : "transparent"
        Text { anchors.centerIn: parent; text: "−"; font.pixelSize: 18; color: control.down.enabled ? control.design.muted : control.design.disabledText }
    }
    up.indicator: Rectangle {
        x: control.width-width-1; y: 1; width: 24; height: control.height-2
        radius: 3; color: control.up.pressed ? control.design.accentLight : control.up.hovered ? control.design.surface : "transparent"
        Text { anchors.centerIn: parent; text: "+"; font.pixelSize: 17; color: control.up.enabled ? control.design.muted : control.design.disabledText }
    }
}
