/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Dialog {
    id: bandMenu

    property int index: 0
    property var menuButton: null
    required property var bandDB

    title: `${i18n("Band")} ${bandMenu.index + 1}` // qmllint disable

    onClosed: {
        menuButton.checked = false;
    }

    footer: RowLayout {
        FormCard.FormComboBoxDelegate {
            id: bandType

            readonly property string bandName: `band${bandMenu.index}Type`

            Layout.margins: Kirigami.Units.smallSpacing
            text: i18n("Type") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: bandMenu.bandDB[bandName]
            editable: false
            model: [i18n("Off"), i18n("Bell"), i18n("High-pass"), i18n("High-shelf"), i18n("Low-pass"), i18n("Low-shelf"), i18n("Notch"), i18n("Resonance"), i18n("Allpass"), i18n("Bandpass"), i18n("Ladder-pass"), i18n("Ladder-rejection")] // qmllint disable
            onActivated: idx => {
                bandMenu.bandDB[bandName] = idx;
            }
        }

        FormCard.FormComboBoxDelegate {
            readonly property string bandName: `band${bandMenu.index}Mode`

            Layout.margins: Kirigami.Units.smallSpacing
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
            readonly property string bandName: `band${bandMenu.index}Slope`

            Layout.margins: Kirigami.Units.smallSpacing
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

        spacing: 0

        RowLayout {
            Layout.margins: Kirigami.Units.smallSpacing

            EeSpinBox {
                readonly property string bandName: `band${bandMenu.index}Frequency`
                label: i18n("Frequency") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 0
                stepSize: 1
                unit: Units.hz
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty(`band${bandMenu.index}Frequency`)
            }
        }

        RowLayout {
            Layout.margins: Kirigami.Units.smallSpacing

            EeSpinBox {
                readonly property string bandName: `band${bandMenu.index}Gain`
                label: i18n("Gain") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 2
                stepSize: 0.01
                unit: Units.dB
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty(`band${bandMenu.index}Gain`)
            }
        }

        RowLayout {
            Layout.margins: Kirigami.Units.smallSpacing

            EeSpinBox {
                readonly property string bandName: `band${bandMenu.index}Q`
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
                onClicked: bandMenu.bandDB.resetProperty(`band${bandMenu.index}Q`)
            }
        }

        RowLayout {
            Layout.margins: Kirigami.Units.smallSpacing

            EeSpinBox {
                readonly property string bandName: `band${bandMenu.index}Width`
                enabled: (bandType.currentIndex === 9 || bandType.currentIndex === 10 || bandType.currentIndex === 11) ? true : false
                label: i18n("Width") // qmllint disable
                from: bandMenu.bandDB.getMinValue(bandName)
                to: bandMenu.bandDB.getMaxValue(bandName)
                value: bandMenu.bandDB[bandName]
                decimals: 2
                stepSize: 0.01
                unit: i18n("oct")
                onValueModified: v => {
                    bandMenu.bandDB[bandName] = v;
                }
            }

            Controls.Button {
                icon.name: "edit-reset-symbolic"
                onClicked: bandMenu.bandDB.resetProperty(`band${bandMenu.index}Width`)
            }
        }

        EeSwitch {
            readonly property string bandName: `band${bandMenu.index}Mute`

            Layout.margins: Kirigami.Units.smallSpacing
            label: i18n("Mute") // qmllint disable
            isChecked: bandMenu.bandDB[bandName]
            onCheckedChanged: {
                if (isChecked !== bandMenu.bandDB[bandName])
                    bandMenu.bandDB[bandName] = isChecked;
            }
        }

        EeSwitch {
            readonly property string bandName: `band${bandMenu.index}Solo`

            Layout.margins: Kirigami.Units.smallSpacing
            label: i18n("Solo") // qmllint disable
            isChecked: bandMenu.bandDB[bandName]
            onCheckedChanged: {
                if (isChecked !== bandMenu.bandDB[bandName])
                    bandMenu.bandDB[bandName] = isChecked;
            }
        }
    }
}
