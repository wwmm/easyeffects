import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: control

    property alias isChecked: qtSwitch.checked
    property string label: ""
    property string subtitle: ""
    property int maximumLineCount: 2

    onClicked: qtSwitch.toggle()
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.CheckBox
    Accessible.name: control.label
    Accessible.description: control.subtitle
    Accessible.checked: control.checked
    Accessible.onToggleAction: if (control.enabled)
        control.clicked()

    contentItem: RowLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            Label {
                Layout.fillWidth: true
                text: control.label
                elide: Text.ElideRight
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: Text.Wrap
                maximumLineCount: control.maximumLineCount
            }

            Label {
                Layout.fillWidth: true
                text: control.subtitle
                elide: Text.ElideRight
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.Wrap
                maximumLineCount: control.maximumLineCount
                visible: !Common.isEmpty(control.subtitle)
            }
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
