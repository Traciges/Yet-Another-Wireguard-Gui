import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: renameDialog

    required property string profileName

    signal renameAccepted(string newName)

    title: "Rename Profile"
    preferredWidth: Kirigami.Units.gridUnit * 24
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    readonly property bool inputValid: {
        const t = nameField.text.trim()
        return /^[a-zA-Z0-9_=+.\-]{1,15}$/.test(t) && t !== renameDialog.profileName
    }

    Component.onCompleted: {
        const btn = standardButton(Kirigami.Dialog.Ok)
        if (btn)
            btn.enabled = Qt.binding(() => renameDialog.inputValid)
    }

    onOpened: {
        nameField.text = renameDialog.profileName
        nameField.forceActiveFocus()
        nameField.selectAll()
    }

    onAccepted: {
        if (renameDialog.inputValid)
            renameDialog.renameAccepted(nameField.text.trim())
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: Kirigami.Units.largeSpacing
        anchors.rightMargin: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Controls.TextField {
            id: nameField
            Layout.fillWidth: true
            maximumLength: 15
            placeholderText: "new-name  (max 15 chars)"
            Keys.onReturnPressed: { if (renameDialog.inputValid) renameDialog.accept() }
            Keys.onEnterPressed:  { if (renameDialog.inputValid) renameDialog.accept() }
        }

        Controls.Label {
            Layout.fillWidth: true
            visible: nameField.text.trim() !== "" &&
                     nameField.text.trim() !== renameDialog.profileName &&
                     !renameDialog.inputValid
            text: "Only letters, numbers and _ = + . - allowed (max 15 chars)"
            color: Kirigami.Theme.negativeTextColor
            font.pointSize: Kirigami.Theme.smallFont.pointSize
            wrapMode: Text.WordWrap
        }
    }
}
