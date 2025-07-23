import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Controls.Popup {
    id: bandMenu

    property int index: 0
    property var menuButton: null
    required property var bandDB

    parent: menuButton
    focus: true
    x: parent !== null ? Math.round((parent.width - width) / 2) : x
    y: parent !== null ? parent.height : y
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    onClosed: {
        menuButton.checked = false;
    }

    contentItem: ColumnLayout {
        Kirigami.CardsLayout {
            Kirigami.Card {
                actions: [
                    Kirigami.Action {
                        readonly property string bandName: "band" + bandMenu.index + "Mute"
                        text: i18n("Mute")
                        icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-low-symbolic"
                        checkable: true
                        checked: bandMenu.bandDB[bandName]
                        onTriggered: {
                            if (checked != bandMenu.bandDB[bandName])
                                bandMenu.bandDB[bandName] = checked;
                        }
                    },
                    Kirigami.Action {
                        readonly property string bandName: "band" + bandMenu.index + "Solo"
                        text: i18n("Solo")
                        checkable: true
                        icon.name: "starred-symbolic"
                        checked: bandMenu.bandDB[bandName]
                        onTriggered: {
                            if (checked != bandMenu.bandDB[bandName])
                                bandMenu.bandDB[bandName] = checked;
                        }
                    }
                ]
                contentItem: GridLayout {
                    uniformCellWidths: true
                    rowSpacing: Kirigami.Units.largeSpacing
                    columnSpacing: Kirigami.Units.largeSpacing
                    columns: 2
                    FormCard.FormComboBoxDelegate {
                        readonly property string bandName: "band" + bandMenu.index + "Type"
                        text: i18n("Type")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: bandMenu.bandDB[bandName]
                        editable: false
                        model: [i18n("Off"), i18n("Bell"), i18n("Hi-pass"), i18n("Hi-shelf"), i18n("Lo-pass"), i18n("Lo-shelf"), i18n("Notch"), i18n("Resonance"), i18n("Allpass"), i18n("Bandpass"), i18n("Ladder-pass"), i18n("Ladder-rej")]
                        onActivated: idx => {
                            bandMenu.bandDB[bandName] = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        readonly property string bandName: "band" + bandMenu.index + "Mode"
                        text: i18n("Mode")
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
                        text: i18n("Slope")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: bandMenu.bandDB[bandName]
                        editable: false
                        model: [i18n("x1"), i18n("x2"), i18n("x3"), i18n("x4")]
                        onActivated: idx => {
                            bandMenu.bandDB[bandName] = idx;
                        }
                    }

                    EeSpinBox {
                        readonly property string bandName: "band" + bandMenu.index + "Frequency"
                        label: i18n("Frequency")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
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
                }
            }
        }
    }
}
