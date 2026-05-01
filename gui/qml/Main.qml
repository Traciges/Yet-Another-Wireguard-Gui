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
    visible: false

    property string selectedProfile: ""
    property var prevStats: ({})
    property real lastRefreshTime: 0
    property bool firstLoad: true
    property bool daemonUnavailable: false

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

    AddProfileDialog {
        id: addDialog
        onProfileAccepted: (config) => wireguardManager.addProfile(config)
    }

    DeleteProfileDialog {
        id: deleteDialog
        profileName: root.selectedProfile
        onDeleteAccepted: wireguardManager.deleteProfile(root.selectedProfile)
    }

    RenameProfileDialog {
        id: renameDialog
        profileName: root.selectedProfile
        onRenameAccepted: (newName) => wireguardManager.renameProfile(root.selectedProfile, newName)
    }

    SettingsDialog {
        id: settingsDialog
        profileModel: profileModel
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

        function onDaemonUnavailable() {
            root.daemonUnavailable = true
        }

        function onProfilesLoaded(profiles) {
            root.daemonUnavailable = false
            const now = Date.now() / 1000.0;
            const dt = root.lastRefreshTime > 0 ? (now - root.lastRefreshTime) : 0;
            root.lastRefreshTime = now;

            if (root.firstLoad) {
                root.firstLoad = false;
                const autoProfile = settingsManager.autoConnectProfile;
                if (autoProfile !== "") {
                    for (let j = 0; j < profiles.length; j++) {
                        if (profiles[j].name === autoProfile && profiles[j].status === "inactive") {
                            wireguardManager.toggleProfile(autoProfile, true);
                            break;
                        }
                    }
                }
            }

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

        function onProfileRenamed(oldName, newName) {
            if (root.selectedProfile === oldName)
                root.selectedProfile = newName;
            if (settingsManager.autoConnectProfile === oldName)
                settingsManager.setAutoConnectProfile(newName);
            delete root.prevStats[oldName];
            wireguardManager.refreshProfiles();
            root.showPassiveNotification("Profile \"" + oldName + "\" renamed to \"" + newName + "\".");
        }

        function onProfileExported(name) {
            root.showPassiveNotification("Profile \"" + name + "\" exported successfully.");
        }

        function onErrorOccurred(profileName, errorMessage) {
            root.showPassiveNotification((profileName ? profileName + ": " : "") + errorMessage, "long");
        }
    }

    Component.onCompleted: {
        if (!startInTray)
            root.show()
        wireguardManager.refreshProfiles()
    }


    pageStack.initialPage: Kirigami.Page {
        title: "Profiles"

        footer: RowLayout {
            spacing: 0
            anchors.left: parent.left
            anchors.right: parent.right

            Controls.AbstractButton {
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing
                leftPadding: Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.smallSpacing
                contentItem: Image {
                    source: "qrc:/github.svg"
                    sourceSize.width: 16
                    sourceSize.height: 16
                }
                onClicked: Qt.openUrlExternally("https://github.com/Traciges")
                HoverHandler { cursorShape: Qt.PointingHandCursor }
            }

            Controls.Label {
                text: "Made by Guido"
                color: Kirigami.Theme.disabledTextColor
                leftPadding: Kirigami.Units.smallSpacing
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing
            }

            Item { Layout.fillWidth: true }

            Controls.Label {
                text: "v" + Qt.application.version
                color: Kirigami.Theme.disabledTextColor
                rightPadding: 2 * Kirigami.Units.smallSpacing
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing
            }
        }

        actions: [
            Kirigami.Action {
                text: "Settings"
                icon.name: "configure"
                onTriggered: settingsDialog.open()
            },
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
                    exportDialog.currentFile = "file:///" + root.selectedProfile + ".conf";
                    exportDialog.open();
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
                visible: root.daemonUnavailable
                text: "WireGuard daemon is not running"
                explanation: "The yawg-daemon service is disabled.\nClick below to enable and start it."
                icon.name: "dialog-warning"
                helpfulAction: Kirigami.Action {
                    text: "Enable Daemon"
                    icon.name: "system-run"
                    onTriggered: wireguardManager.enableAndStartDaemon()
                }
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                visible: profileModel.count === 0 && !root.daemonUnavailable
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

                delegate: ProfileCard {
                    name: model.name
                    status: model.status
                    rxRate: model.rxRate
                    txRate: model.txRate
                    width: ListView.view.width
                    isSelected: model.name === root.selectedProfile
                    onToggleRequested: state => wireguardManager.toggleProfile(model.name, state)
                    onSelectRequested: root.selectedProfile = (model.name === root.selectedProfile ? "" : model.name)
                    onRenameRequested: {
                        root.selectedProfile = model.name
                        renameDialog.open()
                    }
                }
            }
        }
    }
}
