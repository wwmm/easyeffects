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

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common

Controls.ItemDelegate {
    id: delegate

    required property int index
    required property var bandDB
    required property var menu
    readonly property string bandTypeName: `band${index}Type`

    down: false
    hoverEnabled: false
    height: ListView.view.height

    contentItem: ColumnLayout {

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: delegate.index + 1
            enabled: delegate.bandDB[delegate.bandTypeName] !== 0 // disable when filter is Off
        }

        Controls.Button {
            id: menuButton

            Layout.alignment: Qt.AlignCenter
            icon.name: "emblem-system-symbolic"
            checkable: true
            checked: false
            onCheckedChanged: {
                delegate.menu.index = delegate.index;
                delegate.menu.menuButton = menuButton;
                if (checked)
                    delegate.menu.open();
                else
                    delegate.menu.close();
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: {
                const f = delegate.bandDB[`band${delegate.index}Frequency`];
                if (f < 1000) {
                    return Common.toLocaleLabel(f, 0, Units.hz);
                } else {
                    return Common.toLocaleLabel(f * 0.001, 1, Units.kHz);
                }
            }
            enabled: delegate.bandDB[delegate.bandTypeName] !== 0 // disable when filter is Off
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: "Q " + Common.toLocaleLabel(delegate.bandDB[`band${delegate.index}Q`], 2, "")
            enabled: delegate.bandDB[delegate.bandTypeName] !== 0 // disable when filter is Off
        }

        Controls.Slider {
            id: gainSlider

            readonly property string bandName: `band${delegate.index}Gain`
            property real pageSteps: 10

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true

            orientation: Qt.Vertical
            from: delegate.bandDB.getMinValue(bandName)
            to: delegate.bandDB.getMaxValue(bandName)
            value: delegate.bandDB[bandName]
            stepSize: 0.01
            enabled: true
            onMoved: {
                if (value != delegate.bandDB[bandName])
                    delegate.bandDB[bandName] = value;
            }
            Keys.onPressed: event => {
                if (event.key === Qt.Key_PageUp) {
                    const v = value + pageSteps * stepSize;

                    delegate.bandDB[bandName] = Common.clamp(v, from, to);
                } else if (event.key === Qt.Key_PageDown) {
                    const v = value - pageSteps * stepSize;

                    delegate.bandDB[bandName] = Common.clamp(v, from, to);
                }

                event.accepted = true;
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(gainSlider.value).toLocaleString(Qt.locale(), 'f', 2)
            enabled: false
        }
    }
}
