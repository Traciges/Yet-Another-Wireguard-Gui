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
    property var prevStats: ({})
    property real lastRefreshTime: 0

    function formatRate(bytesPerSec) {
        if (bytesPerSec < 1024)
            return bytesPerSec.toFixed(0) + " B/s";
        if (bytesPerSec < 1048576)
            return (bytesPerSec / 1024).toFixed(1) + " KB/s";
        return (bytesPerSec / 1048576).toFixed(2) + " MB/s";
    }

    Platform.FileDialog {
        id: importDialog
        title: "Import WireGuard Configuration"
        nameFilters: ["WireGuard Configurations (*.conf)"]
        onAccepted: wireguardManager.importProfile(importDialog.file)
    }

    Platform.FileDialog {
        id: exportDialog
        title: "Export WireGuard Configuration"
        fileMode: Platform.FileDialog.SaveFile
        nameFilters: ["WireGuard Configurations (*.conf)"]
        onAccepted: wireguardManager.exportProfile(root.selectedProfile, exportDialog.file)
    }

    Kirigami.Dialog {
        id: addDialog
        title: "Add WireGuard Profile"
        preferredWidth: Kirigami.Units.gridUnit * 32
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

        property bool canAccept: addNameField.text.trim() !== "" && addPrivKeyField.text.trim() !== "" && addAddrField.text.trim() !== "" && addPubKeyField.text.trim() !== "" && addAllowedField.text.trim() !== "" && addEndpointField.text.trim() !== ""

        Component.onCompleted: {
            const btn = standardButton(Kirigami.Dialog.Ok);
            if (btn)
                btn.enabled = Qt.binding(() => addDialog.canAccept);
        }

        onOpened: {
            addNameField.text = "";
            addPrivKeyField.text = "";
            addAddrField.text = "";
            addDnsField.text = "";
            addMtuField.text = "";
            addPubKeyField.text = "";
            addPskField.text = "";
            addAllowedField.text = "";
            addEndpointField.text = "";
            addNameField.forceActiveFocus();
        }

        onAccepted: {
            wireguardManager.addProfile(addNameField.text.trim(), addPrivKeyField.text.trim(), addAddrField.text.trim(), addDnsField.text.trim(), addMtuField.text.trim(), addPubKeyField.text.trim(), addPskField.text.trim(), addAllowedField.text.trim(), addEndpointField.text.trim());
        }

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing

            Kirigami.Heading {
                text: "[Interface]"
                level: 4
                Layout.fillWidth: true
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true

                Controls.TextField {
                    id: addNameField
                    Kirigami.FormData.label: "Name:"
                    placeholderText: "home-vpn  (max 15 chars)"
                    maximumLength: 15
                }
                Controls.TextField {
                    id: addPrivKeyField
                    Kirigami.FormData.label: "Private Key:"
                    placeholderText: "base64-encoded private key"
                }
                Controls.TextField {
                    id: addAddrField
                    Kirigami.FormData.label: "Address:"
                    placeholderText: "10.0.0.2/24"
                }
                Controls.TextField {
                    id: addDnsField
                    Kirigami.FormData.label: "DNS:"
                    placeholderText: "optional, e.g. 192.168.1.1"
                }
                Controls.TextField {
                    id: addMtuField
                    Kirigami.FormData.label: "MTU:"
                    placeholderText: "optional, e.g. 1420"
                    inputMethodHints: Qt.ImhDigitsOnly
                }
            }

            Kirigami.Heading {
                text: "[Peer]"
                level: 4
                Layout.fillWidth: true
            }

            Kirigami.FormLayout {
                Layout.fillWidth: true

                Controls.TextField {
                    id: addPubKeyField
                    Kirigami.FormData.label: "Public Key:"
                    placeholderText: "base64-encoded public key"
                }
                Controls.TextField {
                    id: addPskField
                    Kirigami.FormData.label: "Preshared Key:"
                    placeholderText: "optional"
                }
                Controls.TextField {
                    id: addAllowedField
                    Kirigami.FormData.label: "Allowed IPs:"
                    placeholderText: "0.0.0.0/0, ::/0"
                }
                Controls.TextField {
                    id: addEndpointField
                    Kirigami.FormData.label: "Endpoint:"
                    placeholderText: "vpn.example.com:51820"
                }
            }

            Controls.Label {
                text: "DNS, MTU and Preshared Key are optional - all other fields are required."
                font.pointSize: Kirigami.Theme.smallFont.pointSize
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
        }
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

    ListModel {
        id: profileModel
    }

    Timer {
        id: statsTimer
        interval: 2000
        repeat: true
        running: {
            for (let i = 0; i < profileModel.count; i++) {
                if (profileModel.get(i).status === "active")
                    return true;
            }
            return false;
        }
        onTriggered: wireguardManager.refreshProfiles()
    }

    Connections {
        target: wireguardManager

        function onProfilesLoaded(profiles) {
            const now = Date.now() / 1000.0;
            const dt = root.lastRefreshTime > 0 ? (now - root.lastRefreshTime) : 0;
            root.lastRefreshTime = now;

            const savedSelection = root.selectedProfile;
            profileModel.clear();
            for (let i = 0; i < profiles.length; i++) {
                const p = profiles[i];
                const prev = root.prevStats[p.name];
                let rxRate = 0, txRate = 0;
                if (dt > 0.1 && prev && p.status === "active") {
                    rxRate = Math.max(0, (p.rxBytes - prev.rx) / dt);
                    txRate = Math.max(0, (p.txBytes - prev.tx) / dt);
                }
                root.prevStats[p.name] = {
                    rx: p.rxBytes || 0,
                    tx: p.txBytes || 0
                };
                profileModel.append({
                    name: p.name,
                    status: p.status,
                    rxRate: Math.round(rxRate),
                    txRate: Math.round(txRate)
                });
            }
            root.selectedProfile = savedSelection;
        }

        function onProfileStatusChanged(name, status) {
            for (let i = 0; i < profileModel.count; i++) {
                if (profileModel.get(i).name === name) {
                    profileModel.setProperty(i, "status", status);
                    if (status === "inactive") {
                        profileModel.setProperty(i, "rxRate", 0);
                        profileModel.setProperty(i, "txRate", 0);
                        delete root.prevStats[name];
                    }
                    break;
                }
            }
        }

        function onProfileImported(name) {
            wireguardManager.refreshProfiles();
            root.showPassiveNotification("Profile \"" + name + "\" imported successfully.");
        }

        function onProfileDeleted(name) {
            wireguardManager.refreshProfiles();
            root.showPassiveNotification("Profile \"" + name + "\" deleted.");
        }

        function onProfileExported(name) {
            root.showPassiveNotification("Profile \"" + name + "\" exported successfully.");
        }

        function onErrorOccurred(profileName, errorMessage) {
            root.showPassiveNotification((profileName ? profileName + ": " : "") + errorMessage, "long");
        }
    }

    Component.onCompleted: wireguardManager.refreshProfiles()

    pageStack.initialPage: Kirigami.Page {
        title: "Profiles"

        actions: [
            Kirigami.Action {
                text: "Add"
                icon.name: "list-add"
                onTriggered: addDialog.open()
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
                enabled: root.selectedProfile !== ""
                onTriggered: {
                    exportDialog.currentFile = "file:///" + root.selectedProfile + ".conf"
                    exportDialog.open()
                }
            },
            Kirigami.Action {
                text: "Quit"
                icon.name: "application-exit"
                onTriggered: Qt.quit()
            }
        ]

        Item {
            anchors.fill: parent

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: profileModel.count === 0
                text: "No WireGuard profiles found"
                icon.name: "network-vpn"
            }

            ListView {
                anchors.fill: parent
                anchors.topMargin: Kirigami.Units.smallSpacing
                anchors.bottomMargin: Kirigami.Units.smallSpacing
                visible: profileModel.count > 0
                model: profileModel
                spacing: Kirigami.Units.smallSpacing
                clip: true

                delegate: Kirigami.AbstractCard {
                    id: card
                    required property string name
                    required property string status
                    required property int rxRate
                    required property int txRate

                    width: ListView.view.width
                    highlighted: card.name === root.selectedProfile

                    Rectangle {
                        anchors.fill: parent
                        color: "transparent"
                        border.color: Kirigami.Theme.highlightColor
                        border.width: 2
                        radius: Kirigami.Units.smallSpacing
                        opacity: card.highlighted ? 1.0 : 0.0
                        Behavior on opacity {
                            NumberAnimation {
                                duration: 120
                            }
                        }
                        z: 1
                    }

                    contentItem: RowLayout {
                        spacing: Kirigami.Units.largeSpacing

                        Kirigami.Icon {
                            source: "network-vpn"
                            implicitWidth: Kirigami.Units.iconSizes.medium
                            implicitHeight: Kirigami.Units.iconSizes.medium
                            opacity: card.status === "active" ? 1.0 : 0.4
                            Behavior on opacity {
                                NumberAnimation {
                                    duration: 150
                                }
                            }
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
                                    Behavior on color {
                                        ColorAnimation {
                                            duration: 200
                                        }
                                    }
                                }

                                Controls.Label {
                                    text: card.status === "active" ? "Active" : "Inactive"
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    color: card.status === "active" ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                                }

                                Controls.Label {
                                    visible: card.status === "active"
                                    text: "  ↓ " + root.formatRate(card.rxRate) + "   ↑ " + root.formatRate(card.txRate)
                                    font.pointSize: Kirigami.Theme.smallFont.pointSize
                                    color: Kirigami.Theme.disabledTextColor
                                    opacity: (card.rxRate > 0 || card.txRate > 0) ? 1.0 : 0.5
                                }
                            }
                        }

                        Controls.Switch {
                            checked: card.status === "active"
                            onToggled: wireguardManager.toggleProfile(card.name, checked)
                        }
                    }

                    HoverHandler {
                        cursorShape: Qt.PointingHandCursor
                    }

                    TapHandler {
                        onTapped: root.selectedProfile = card.name === root.selectedProfile ? "" : card.name
                    }
                }
            }
        }
    }
}
