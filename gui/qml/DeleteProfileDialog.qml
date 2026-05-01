import org.kde.kirigami as Kirigami

Kirigami.PromptDialog {
    id: deleteDialog

    required property string profileName

    signal deleteAccepted()

    title: "Delete Profile"
    subtitle: "Are you sure you want to delete the profile \"" + deleteDialog.profileName + "\"?"
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    onAccepted: deleteAccepted()
}
