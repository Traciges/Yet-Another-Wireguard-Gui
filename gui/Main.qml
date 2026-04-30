import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Qt.labs.platform as Platform

Kirigami.ApplicationWindow {
    id: root
    title: "Yet Another Wireguard Gui"
    width: 450
    height: 800

    property string selectedProfile: ""

    Platform.FileDialog {
        id: importDialog
        title: "Import WireGuard Configuration"
        nameFilters: ["WireGuard Configurations (*.conf)"]
        onAccepted: wireguardManager.importProfile(importDialog.file)
    }

    Kirigami.PromptDialog {
        id: deleteDialog
        title: "Delete Profile"
        subtitle: "Are you sure you want to delete the profile \"" + root.selectedProfile + "\"?"
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: wireguardManager.deleteProfile(root.selectedProfile)
    }

    Platform.SystemTrayIcon {
        visible: true
        icon.source: "qrc:/app-icon.png"
        tooltip: "Yet Another Wireguard Gui"
        onActivated: root.visible ? root.hide() : root.show()
    }

    ListModel { id: profileModel }

    Connections {
        target: wireguardManager
        function onProfilesLoaded(profiles) {
            profileModel.clear()
            root.selectedProfile = ""
            for (let i = 0; i < profiles.length; i++) {
                profileModel.append(profiles[i])
            }
        }
        function onProfileStatusChanged(name, status) {
            for (let i = 0; i < profileModel.count; i++) {
                if (profileModel.get(i).name === name) {
                    profileModel.setProperty(i, "status", status)
                    break
                }
            }
        }
        function onProfileImported(name) {
            wireguardManager.refreshProfiles()
            root.showPassiveNotification("Profile \"" + name + "\" imported successfully.")
        }
        function onProfileDeleted(name) {
            wireguardManager.refreshProfiles()
            root.showPassiveNotification("Profile \"" + name + "\" deleted.")
        }
        function onErrorOccurred(profileName, errorMessage) {
            root.showPassiveNotification(
                (profileName ? profileName + ": " : "") + errorMessage, "long"
            )
        }
    }

    Component.onCompleted: wireguardManager.refreshProfiles()

    pageStack.initialPage: Kirigami.Page {
        title: "Profiles"

        actions: [
            Kirigami.Action {
                text: "Add"
                icon.name: "list-add"
                onTriggered: console.log("Add profile (not implemented)")
            },
            Kirigami.Action {
                text: "Remove"
                icon.name: "list-remove"
                enabled: root.selectedProfile !== ""
                onTriggered: deleteDialog.open()
            },
            Kirigami.Action {
                text: "Import"
                icon.name: "document-import"
                onTriggered: importDialog.open()
            },
            Kirigami.Action {
                text: "Export"
                icon.name: "document-export"
                onTriggered: root.showPassiveNotification("Not implemented yet")
            },
            Kirigami.Action {
                text: "Quit"
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        ]

        Kirigami.CardsListView {
            anchors.fill: parent
            model: profileModel

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: profileModel.count === 0
                text: "No WireGuard profiles found"
                icon.name: "network-vpn"
            }

            delegate: Kirigami.AbstractCard {
                id: card
                required property string name
                required property string status

                highlighted: card.name === root.selectedProfile

                HoverHandler {
                    id: hoverHandler
                    cursorShape: Qt.PointingHandCursor
                }

                Rectangle {
                    anchors.fill: parent
                    color: "transparent"
                    border.color: Kirigami.Theme.highlightColor
                    border.width: 2
                    radius: Kirigami.Units.smallSpacing
                    opacity: card.highlighted ? 1.0 : 0.0
                    Behavior on opacity { NumberAnimation { duration: 120 } }
                    z: 1
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Heading {
                            level: 3
                            text: card.name
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            width: badge.implicitWidth + Kirigami.Units.smallSpacing * 3
                            height: badge.implicitHeight + Kirigami.Units.smallSpacing
                            radius: 4
                            color: card.status === "active" ? "#4caf50" : "#757575"

                            Controls.Label {
                                id: badge
                                anchors.centerIn: parent
                                text: card.status === "active" ? "Active" : "Inactive"
                                color: "white"
                                font.bold: true
                                font.pointSize: 8
                            }
                        }
                    }

                    Controls.Switch {
                        checked: card.status === "active"
                        onToggled: wireguardManager.toggleProfile(card.name, checked)
                    }
                }

                TapHandler {
                    onTapped: root.selectedProfile = card.name === root.selectedProfile ? "" : card.name
                }
            }
        }
    }
}
