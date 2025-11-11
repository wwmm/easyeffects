import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.OverlaySheet {
    id: bandMenu

    property int index: 0
    property var menuButton: null
    required property var bandDB

    parent: applicationWindow().overlay// qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit// qmllint disable
    implicitWidth: Math.min(columnLayout.implicitWidth, appWindow.width * 0.8) + 4 * Kirigami.Units.iconSizes.large// qmllint disable
    implicitHeight: Math.min(bandMenu.header.height + bandMenu.footer.height + 1.2 * columnLayout.implicitHeight, bandMenu.parent.height - (bandMenu.header.height + bandMenu.footer.height) - bandMenu.y)

    onClosed: {
        menuButton.checked = false;
    }

    header: Controls.Label {
        text: i18n("Band") + ` ${bandMenu.index + 1}` // qmllint disable
    }

    footer: RowLayout {
        FormCard.FormComboBoxDelegate {
            id: bandType

            readonly property string bandName: "band" + bandMenu.index + "Type"
            text: i18n("Type") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: bandMenu.bandDB[bandName]
            editable: false
            model: [i18n("Off"), i18n("Bell"), i18n("High-pass"), i18n("High-shelf"), i18n("Low-pass"), i18n("Low-shelf"), i18n("Notch"), i18n("Resonance"), i18n("Allpass"), i18n("Bandpass"), i18n("Ladder-pass"), i18n("Ladder-rejection")]// qmllint disable
            onActivated: idx => {
                bandMenu.bandDB[bandName] = idx;
            }
        }

        FormCard.FormComboBoxDelegate {
            readonly property string bandName: "band" + bandMenu.index + "Mode"
            text: i18n("Mode") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: bandMenu.bandDB[bandName]
            editable: false
            model: [i18n("RLC (BT)"), i18n("RLC (MT)"), i18n("BWC (BT)"), i18n("BWC (MT)"), i18n("LRX (BT)"), i18n("LRX (MT)"), i18n("APO (DR)")]
            onActivated: idx => {
                bandMenu.bandDB[bandName] = idx;
            }
        }

        FormCard.FormComboBoxDelegate {
            readonly property string bandName: "band" + bandMenu.index + "Slope"
            text: i18n("Slope") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: bandMenu.bandDB[bandName]
            editable: false
            model: ["x1", "x2", "x3", "x4"]
            onActivated: idx => {
                bandMenu.bandDB[bandName] = idx;
            }
        }
    }

    ColumnLayout {
        id: columnLayout

        RowLayout {
            EeSpinBox {
                readonly property string bandName: "band" + bandMenu.index + "Frequency"
                label: i18n("Frequency") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 0
                stepSize: 1
                unit: "Hz"
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty("band" + bandMenu.index + "Frequency")
            }
        }

        RowLayout {
            EeSpinBox {
                readonly property string bandName: "band" + bandMenu.index + "Gain"
                label: i18n("Gain") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 2
                stepSize: 0.01
                unit: "dB"
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty("band" + bandMenu.index + "Gain")
            }
        }

        RowLayout {
            EeSpinBox {
                readonly property string bandName: "band" + bandMenu.index + "Q"
                label: i18n("Quality") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 2
                stepSize: 0.01
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty("band" + bandMenu.index + "Q")
            }
        }

        RowLayout {
            EeSpinBox {
                readonly property string bandName: "band" + bandMenu.index + "Width"
                enabled: (bandType.currentIndex === 9 || bandType.currentIndex === 10 || bandType.currentIndex === 11) ? true : false
                label: i18n("Width") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 2
                stepSize: 0.01
                unit: "oct"
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty("band" + bandMenu.index + "Width")
            }
        }

        EeSwitch {
            readonly property string bandName: "band" + bandMenu.index + "Mute"
            label: i18n("Mute") // qmllint disable
            isChecked: bandMenu.bandDB[bandName]
            onCheckedChanged: {
                if (isChecked !== bandMenu.bandDB[bandName])
                    bandMenu.bandDB[bandName] = isChecked;
            }
        }

        EeSwitch {
            readonly property string bandName: "band" + bandMenu.index + "Solo"
            label: i18n("Solo") // qmllint disable
            isChecked: bandMenu.bandDB[bandName]
            onCheckedChanged: {
                if (isChecked !== bandMenu.bandDB[bandName])
                    bandMenu.bandDB[bandName] = isChecked;
            }
        }
    }
}
