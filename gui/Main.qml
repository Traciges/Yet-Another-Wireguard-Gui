import QtQuick
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: root
    title: "Yet Another Wireguard GUI"
    width: 500
    height: 700

    Kirigami.Page {
        title: "Dashboard"

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            text: "Hello Wireguard"
        }
    }
}
