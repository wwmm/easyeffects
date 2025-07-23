import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Controls.ItemDelegate {
    id: listItemDelegate

    required property int index
    required property var bandDB

    function toLocaleLabel(num, decimal, unit) {
        return Number(num).toLocaleString(Qt.locale(), 'f', decimal) + ` ${unit}`;
    }

    down: false
    hoverEnabled: false
    height: ListView.view.height

    Controls.Popup {
        id: menu

        parent: menuButton
        focus: true
        x: Math.round((parent.width - width) / 2)
        y: parent.height
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
        onClosed: {
            menuButton.checked = false;
        }

        contentItem: ColumnLayout {
            Kirigami.CardsLayout {
                Kirigami.Card {
                    actions: [
                        Kirigami.Action {
                            readonly property string bandName: "band" + index + "Mute"
                            text: i18n("Mute")
                            icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-low-symbolic"
                            checkable: true
                            checked: bandDB[bandName]
                            onTriggered: {
                                if (checked != bandDB[bandName])
                                    bandDB[bandName] = checked;
                            }
                        },
                        Kirigami.Action {
                            readonly property string bandName: "band" + index + "Solo"
                            text: i18n("Solo")
                            checkable: true
                            icon.name: "starred-symbolic"
                            checked: bandDB[bandName]
                            onTriggered: {
                                if (checked != bandDB[bandName])
                                    bandDB[bandName] = checked;
                            }
                        }
                    ]
                    contentItem: GridLayout {
                        uniformCellWidths: true
                        rowSpacing: Kirigami.Units.largeSpacing
                        columnSpacing: Kirigami.Units.largeSpacing
                        columns: 2
                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + index + "Type"
                            text: i18n("Type")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandDB[bandName]
                            editable: false
                            model: [i18n("Off"), i18n("Bell"), i18n("Hi-pass"), i18n("Hi-shelf"), i18n("Lo-pass"), i18n("Lo-shelf"), i18n("Notch"), i18n("Resonance"), i18n("Allpass"), i18n("Bandpass"), i18n("Ladder-pass"), i18n("Ladder-rej")]
                            onActivated: idx => {
                                bandDB[bandName] = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + index + "Mode"
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandDB[bandName]
                            editable: false
                            model: [i18n("RLC (BT)"), i18n("RLC (MT)"), i18n("BWC (BT)"), i18n("BWC (MT)"), i18n("LRX (BT)"), i18n("LRX (MT)"), i18n("APO (DR)")]
                            onActivated: idx => {
                                bandDB[bandName] = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + index + "Slope"
                            text: i18n("Slope")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandDB[bandName]
                            editable: false
                            model: [i18n("x1"), i18n("x2"), i18n("x3"), i18n("x4")]
                            onActivated: idx => {
                                bandDB[bandName] = idx;
                            }
                        }

                        EeSpinBox {
                            readonly property string bandName: "band" + index + "Frequency"
                            label: i18n("Frequency")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: bandDB.getMinValue(bandName)
                            to: bandDB.getMaxValue(bandName)
                            value: bandDB[bandName]
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            onValueModified: v => {
                                bandDB[bandName] = v;
                            }
                        }
                    }
                }
            }
        }
    }

    contentItem: ColumnLayout {
        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: index + 1
            enabled: false
        }

        Controls.Button {
            id: menuButton

            Layout.alignment: Qt.AlignCenter
            icon.name: "emblem-system-symbolic"
            checkable: true
            checked: false
            onCheckedChanged: {
                if (checked)
                    menu.open();
                else
                    menu.close();
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: {
                const f = bandDB["band" + index + "Frequency"];
                if (f < 1000) {
                    return toLocaleLabel(f, 0, "Hz");
                } else {
                    toLocaleLabel(f * 0.001, 1, "kHz");
                }
            }
            enabled: false
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Q") + " " + toLocaleLabel(bandDB["band" + index + "Q"], 2, "")
            enabled: false
        }

        Controls.Slider {
            id: gainSlider

            readonly property string bandName: "band" + index + "Gain"

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: bandDB.getMinValue(bandName)
            to: bandDB.getMaxValue(bandName)
            value: bandDB[bandName]
            stepSize: 1
            enabled: true
            onMoved: {
                if (value != bandDB[bandName])
                    bandDB[bandName] = value;
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(gainSlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }
    }
}
