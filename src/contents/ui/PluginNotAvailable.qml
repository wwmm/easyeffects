import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: control

    required property string translatedName
    required property string packageName

    Kirigami.PlaceholderMessage {
        // We need to specify the width here, otherwise the text won't be
        // wrapped on window restricted sizes.
        width: parent.width
        anchors.centerIn: parent
        text: i18n("Effect Not Available")// qmllint disable
        explanation: i18n("%1 cannot be used because %2 is not installed on the system.", translatedName, `<strong>${control.packageName}</strong>`)// qmllint disable
        icon.name: "applications-system-symbolic"
    }
}
