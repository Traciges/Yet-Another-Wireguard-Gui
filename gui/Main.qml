import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import Qt.labs.platform as Platform

Kirigami.ApplicationWindow {
    id: root
    title: "Yet Another Wireguard GUI"
    width: 520
    height: 720

    Platform.SystemTrayIcon {
        visible: true
        icon.source: "qrc:/app-icon.png"
        tooltip: "Yet Another Wireguard GUI"
        onActivated: root.visible ? root.hide() : root.show()
    }

    ListModel { id: profileModel }

    Connections {
        target: wireguardManager
        function onProfilesLoaded(profiles) {
            profileModel.clear()
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
                onTriggered: console.log("Remove profile (not implemented)")
            },
            Kirigami.Action {
                text: "Import"
                icon.name: "document-import"
                onTriggered: root.showPassiveNotification("Not implemented yet")
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
            }
        }
    }
}
