import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: addDialog
    title: "Add WireGuard Profile"
    preferredWidth: Kirigami.Units.gridUnit * 32
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    signal profileAccepted(
        string name, string privateKey, string address,
        string dns, string mtu, string publicKey,
        string presharedKey, string allowedIPs, string endpoint
    )

    property bool canAccept:
        nameField.text.trim() !== "" &&
        privKeyField.text.trim() !== "" &&
        addrField.text.trim() !== "" &&
        pubKeyField.text.trim() !== "" &&
        allowedField.text.trim() !== "" &&
        endpointField.text.trim() !== ""

    Component.onCompleted: {
        const btn = standardButton(Kirigami.Dialog.Ok);
        if (btn)
            btn.enabled = Qt.binding(() => addDialog.canAccept);
    }

    onOpened: {
        nameField.text = "";
        privKeyField.text = "";
        addrField.text = "";
        dnsField.text = "";
        mtuField.text = "";
        pubKeyField.text = "";
        pskField.text = "";
        allowedField.text = "";
        endpointField.text = "";
        nameField.forceActiveFocus();
    }

    onAccepted: profileAccepted(
        nameField.text.trim(), privKeyField.text.trim(), addrField.text.trim(),
        dnsField.text.trim(), mtuField.text.trim(), pubKeyField.text.trim(),
        pskField.text.trim(), allowedField.text.trim(), endpointField.text.trim()
    )

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
                id: nameField
                Kirigami.FormData.label: "Name:"
                placeholderText: "home-vpn  (max 15 chars)"
                maximumLength: 15
            }
            Controls.TextField {
                id: privKeyField
                Kirigami.FormData.label: "Private Key:"
                placeholderText: "base64-encoded private key"
            }
            Controls.TextField {
                id: addrField
                Kirigami.FormData.label: "Address:"
                placeholderText: "10.0.0.2/24"
            }
            Controls.TextField {
                id: dnsField
                Kirigami.FormData.label: "DNS:"
                placeholderText: "optional, e.g. 192.168.1.1"
            }
            Controls.TextField {
                id: mtuField
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
                id: pubKeyField
                Kirigami.FormData.label: "Public Key:"
                placeholderText: "base64-encoded public key"
            }
            Controls.TextField {
                id: pskField
                Kirigami.FormData.label: "Preshared Key:"
                placeholderText: "optional"
            }
            Controls.TextField {
                id: allowedField
                Kirigami.FormData.label: "Allowed IPs:"
                placeholderText: "0.0.0.0/0, ::/0"
            }
            Controls.TextField {
                id: endpointField
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
