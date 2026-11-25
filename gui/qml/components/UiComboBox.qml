import QtQuick
import QtQuick.Controls

ComboBox {
    id: control
    required property var design
    implicitHeight: design.controlHeight
    leftPadding: 10; rightPadding: 28
    hoverEnabled: true; font.pixelSize: 13
    contentItem: Text {
        text: control.displayText; font: control.font
        color: control.enabled ? control.design.text : control.design.disabledText
        verticalAlignment: Text.AlignVCenter; elide: Text.ElideRight
    }
    indicator: Item {
        x: control.width-23; y: (control.height-height)/2; width: 12; height: 8
        Rectangle { x: 1; y: 2; width: 7; height: 1.5; rotation: 45; color: control.design.muted }
        Rectangle { x: 5; y: 2; width: 7; height: 1.5; rotation: -45; color: control.design.muted }
    }
    background: Rectangle {
        color: control.enabled ? (control.hovered ? control.design.surface : control.design.panel) : control.design.surface
        radius: control.design.radius
        border.color: control.activeFocus ? control.design.accent : control.design.strongBorder
        border.width: control.activeFocus ? 2 : 1
    }
    delegate: ItemDelegate {
        required property int index
        required property var modelData
        width: control.width; height: 34
        highlighted: control.highlightedIndex === index
        contentItem: Text { text: modelData; color: control.design.text; font: control.font; verticalAlignment: Text.AlignVCenter }
        background: Rectangle { color: parent.highlighted ? control.design.accentLight : control.design.panel }
    }
    popup: Popup {
        y: control.height+3; width: control.width; padding: 4
        implicitHeight: contentItem.implicitHeight+8
        contentItem: ListView {
            implicitHeight: contentHeight; clip: true
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }
        background: Rectangle { color: control.design.panel; border.color: control.design.strongBorder; radius: control.design.radius }
    }
}
