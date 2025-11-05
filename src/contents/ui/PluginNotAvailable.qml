import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: control

    required property string packageName

    Kirigami.PlaceholderMessage {

        anchors.centerIn: parent
        text: i18n("Not available")// qmllint disable
        explanation: i18n("This plugin requires: %1", control.packageName)// qmllint disable
        icon.name: "applications-system-symbolic"
    }
}
