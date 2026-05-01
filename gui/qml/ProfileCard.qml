import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: card

    property string name: ""
    property string status: ""
    property int rxRate: 0
    property int txRate: 0
    property bool isSelected: false

    signal toggleRequested(bool targetState)
    signal selectRequested()
    signal renameRequested()

    highlighted: card.isSelected

    function formatRate(bytesPerSec) {
        if (bytesPerSec < 1024)
            return bytesPerSec.toFixed(0) + " B/s";
        if (bytesPerSec < 1048576)
            return (bytesPerSec / 1024).toFixed(1) + " KB/s";
        return (bytesPerSec / 1048576).toFixed(2) + " MB/s";
    }

    Rectangle {
        anchors.fill: parent
        color: "transparent"
        border.color: Kirigami.Theme.highlightColor
        border.width: 2
        radius: Kirigami.Units.smallSpacing
        opacity: card.isSelected ? 1.0 : 0.0
        Behavior on opacity { NumberAnimation { duration: 120 } }
        z: 1
    }

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            source: "network-vpn"
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium
            opacity: card.status === "active" ? 1.0 : 0.4
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 3

            Kirigami.Heading {
                level: 3
                text: card.name
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            RowLayout {
                spacing: Kirigami.Units.smallSpacing

                Rectangle {
                    width: 8
                    height: 8
                    radius: 4
                    color: card.status === "active" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                    Behavior on color { ColorAnimation { duration: 200 } }
                }

                Controls.Label {
                    text: card.status === "active" ? "Active" : "Inactive"
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: card.status === "active" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                }

                Controls.Label {
                    visible: card.status === "active"
                    text: "  ↓ " + card.formatRate(card.rxRate) + "   ↑ " + card.formatRate(card.txRate)
                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                    color: Kirigami.Theme.disabledTextColor
                    opacity: (card.rxRate > 0 || card.txRate > 0) ? 1.0 : 0.5
                }
            }
        }

        Controls.ToolButton {
            icon.name: "edit-rename"
            opacity: (cardHoverHandler.hovered && card.status !== "active") ? 1.0 : 0.0
            visible: card.status !== "active"
            Behavior on opacity { NumberAnimation { duration: 120 } }
            Controls.ToolTip.visible: hovered
            Controls.ToolTip.text: "Rename"
            Controls.ToolTip.delay: 500
            onClicked: card.renameRequested()
        }

        Controls.Switch {
            checked: card.status === "active"
            onToggled: card.toggleRequested(checked)
        }
    }

    HoverHandler {
        id: cardHoverHandler
        cursorShape: Qt.PointingHandCursor
    }

    TapHandler {
        onTapped: card.selectRequested()
    }
}
