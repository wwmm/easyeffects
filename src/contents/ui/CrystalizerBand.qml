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
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: delegate

    required property int index
    required property real adaptiveIntensity
    required property var pluginDB
    required property var pluginBackend
    readonly property real frequency: pluginBackend?.getBandFrequency(index) ?? 0.0

    down: false
    hoverEnabled: false
    height: ListView.view.height

    onAdaptiveIntensityChanged: {
        levelMeter.setValue(delegate.adaptiveIntensity);
    }

    Controls.Popup {
        id: menu

        parent: menuButton
        focus: true
        x: Math.round((parent.width - width) / 2)
        y: parent.height
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnReleaseOutside
        onClosed: {
            menuButton.checked = false;
        }

        contentItem: ColumnLayout {
            Controls.Button {
                Layout.alignment: Qt.AlignCenter
                text: i18n("Mute") // qmllint disable
                checkable: true
                checked: delegate.pluginDB["muteBand" + delegate.index]
                onCheckedChanged: {
                    if (checked != delegate.pluginDB["muteBand" + delegate.index])
                        delegate.pluginDB["muteBand" + delegate.index] = checked;
                }
            }

            Controls.Button {
                Layout.alignment: Qt.AlignCenter
                text: i18n("Bypass") // qmllint disable
                checkable: true
                checked: delegate.pluginDB["bypassBand" + delegate.index]
                onCheckedChanged: {
                    if (checked != delegate.pluginDB["bypassBand" + delegate.index])
                        delegate.pluginDB["bypassBand" + delegate.index] = checked;
                }
            }
        }
    }

    contentItem: ColumnLayout {
        anchors.fill: parent

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
            text: frequency >= 1000 ? Common.toLocaleLabel(delegate.frequency / 1000, 1, Units.kHz) : Common.toLocaleLabel(delegate.frequency, 0, Units.hz)
            enabled: false
        }

        Controls.Slider {
            id: intensitySlider

            property real pageSteps: 10

            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            orientation: Qt.Vertical
            from: delegate.pluginDB.getMinValue("intensityBand" + delegate.index)
            to: delegate.pluginDB.getMaxValue("intensityBand" + delegate.index)
            value: delegate.pluginDB["intensityBand" + delegate.index]
            stepSize: 1
            enabled: !delegate.pluginDB["muteBand" + delegate.index] && !delegate.pluginDB["bypassBand" + delegate.index]
            onMoved: {
                if (value != delegate.pluginDB["intensityBand" + delegate.index])
                    delegate.pluginDB["intensityBand" + delegate.index] = value;
            }
            Keys.onPressed: event => {
                if (event.key === Qt.Key_PageUp) {
                    const v = value + pageSteps * stepSize;

                    delegate.pluginDB["intensityBand" + delegate.index] = Common.clamp(v, from, to);

                    event.accepted = true;
                } else if (event.key === Qt.Key_PageDown) {
                    const v = value - pageSteps * stepSize;

                    delegate.pluginDB["intensityBand" + delegate.index] = Common.clamp(v, from, to);

                    event.accepted = true;
                }
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: Number(intensitySlider.value).toLocaleString(Qt.locale(), 'f', 0)
            enabled: false
        }

        EeAudioLevel {
            id: levelMeter

            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            implicitWidth: 2.5 * Kirigami.Units.gridUnit
            implicitHeight: 2.5 * Kirigami.Units.gridUnit
            from: delegate.pluginDB.getMinValue("intensityBand" + delegate.index)
            to: delegate.pluginDB.getMaxValue("intensityBand" + delegate.index)
            decimals: 1
            visible: delegate.pluginDB.adaptiveIntensity
        }
    }
}
