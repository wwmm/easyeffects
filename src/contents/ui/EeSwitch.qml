import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: control

    property alias isChecked: qtSwitch.checked
    property string label: ""

    onClicked: qtSwitch.toggle()
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    contentItem: RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        Label {
            Layout.fillWidth: true
            text: control.label
            elide: Text.ElideRight
            color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            wrapMode: Text.Wrap
            maximumLineCount: 2
            Accessible.ignored: true
        }

        Switch {
            id: qtSwitch

            onCheckedChanged: {
                control.checkedChanged();
            }
            Layout.leftMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            enabled: control.enabled
        }

    }

}
