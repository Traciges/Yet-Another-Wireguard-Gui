import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.Dialog {
    id: settingsDialog

    property var profileModel

    title: "Settings"
    modal: true
    standardButtons: Controls.Dialog.Save | Controls.Dialog.Cancel

    anchors.centerIn: Overlay.overlay
    width: Math.min(parent.width - 2 * Kirigami.Units.largeSpacing, 420)

    onOpened: {
        autostartSwitch.checked = settingsManager.autostartEnabled
        updateComboSelection()
    }

    onAccepted: {
        settingsManager.setAutostartEnabled(autostartSwitch.checked)
        if (autostartSwitch.checked) {
            const idx = profileCombo.currentIndex
            settingsManager.setAutoConnectProfile(
                idx > 0 ? profileCombo.model[idx] : ""
            )
        }
    }

    function updateComboSelection() {
        const saved = settingsManager.autoConnectProfile
        if (saved === "") {
            profileCombo.currentIndex = 0
            return
        }
        for (let i = 1; i < profileCombo.model.length; i++) {
            if (profileCombo.model[i] === saved) {
                profileCombo.currentIndex = i
                return
            }
        }
        profileCombo.currentIndex = 0
    }

    function buildProfileList() {
        const names = ["— None —"]
        for (let i = 0; i < settingsDialog.profileModel.count; i++) {
            names.push(settingsDialog.profileModel.get(i).name)
        }
        return names
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "system-run"
                width: Kirigami.Units.iconSizes.small
                height: Kirigami.Units.iconSizes.small
            }

            Controls.Label {
                text: "Launch at system startup"
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
            }

            Controls.Switch {
                id: autostartSwitch
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            enabled: autostartSwitch.checked
            opacity: autostartSwitch.checked ? 1.0 : 0.4

            Controls.Label {
                text: "Auto-connect on startup"
                font.bold: true
            }

            Controls.Label {
                text: "Automatically connect to a VPN profile when the app starts."
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
            }

            Controls.ComboBox {
                id: profileCombo
                Layout.fillWidth: true
                model: settingsDialog.buildProfileList()
            }
        }
    }
}
