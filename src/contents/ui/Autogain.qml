import EEtagsPluginName
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: autogainPage

    anchors.fill: parent

    Kirigami.PlaceholderMessage {
        anchors.centerIn: parent
        text: i18n("Autogain")
        explanation: i18n("todo")
        icon.name: "emblem-music-symbolic"
    }

}
