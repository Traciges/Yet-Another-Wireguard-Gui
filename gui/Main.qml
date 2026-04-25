import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: "Yet Another Wireguard GUI"
    width: 800
    height: 600

    Kirigami.Page {
        title: "Dashboard"

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Hello Wireguard"
        }
    }
}
