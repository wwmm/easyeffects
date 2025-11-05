import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: control

    required property string translatedName
    required property string packageName

    Kirigami.PlaceholderMessage {

        anchors.centerIn: parent
        text: i18n("Effect Not Available")// qmllint disable
        explanation: i18n("%1 cannot be used because %2 is not installed on the system.", translatedName, `<strong>${control.packageName}</strong>`)// qmllint disable
        icon.name: "applications-system-symbolic"
    }
}
