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
import ee.pipewire as PW
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    id: root

    required property var model
    required property bool connected
    required property bool isBlocklisted
    required property bool mute
    required property int id
    required property int serial
    required property int nVolumeChannels
    required property real volume
    required property string name
    required property string appIconName
    required property string mediaName
    required property string mediaClass
    required property string state
    required property string format
    required property real rate
    required property real latency

    visible: {
        if (!isBlocklisted)
            return true;

        if (mediaClass === "Stream/Output/Audio")
            return DbStreamOutputs.showBlocklistedApps;
        else if (mediaClass === "Stream/Input/Audio")
            return DbStreamInputs.showBlocklistedApps;
    }

    contentItem: ColumnLayout {
        anchors.fill: parent

        Kirigami.CardsLayout {
            id: delegateLayout

            maximumColumns: 3
            maximumColumnWidth: root.parent?.width / maximumColumns

            Kirigami.Icon {
                Layout.fillHeight: true
                source: root.appIconName
                fallback: "folder-sound-symbolic"
            }

            ColumnLayout {
                Layout.columnSpan: delegateLayout.columns === 3 ? 1 : delegateLayout.columns

                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 2
                    text: root.name
                }

                Controls.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: root.mediaName
                    color: Kirigami.Theme.disabledTextColor
                }

                RowLayout {
                    Controls.Label {
                        Layout.fillWidth: true
                        wrapMode: Text.WordWrap
                        text: {
                            const rate = root.rate.toLocaleString(Qt.locale(), 'f', 1);
                            const latency = root.latency.toLocaleString(Qt.locale(), 'f', 1);

                            return `${root.state} · ${root.format} · ${rate} ${Units.kHz} · ${root.nVolumeChannels} ${i18n("channels")} · ${latency} ${Units.ms}`; // qmllint disable
                        }
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
            }

            ColumnLayout {
                Layout.alignment: Qt.AlignTop

                Controls.CheckBox {
                    text: i18n("Enable") // qmllint disable
                    checked: root.connected
                    visible: {
                        if (root.mediaClass === "Stream/Output/Audio")
                            return !DbMain.processAllOutputs;
                        else if (root.mediaClass === "Stream/Input/Audio")
                            return !DbMain.processAllInputs;
                    }
                    onCheckedChanged: {
                        if (checked === true && !root.isBlocklisted) {
                            if (root.mediaClass === "Stream/Output/Audio")
                                PW.Manager.connectStreamOutput(root.id);
                            else if (root.mediaClass === "Stream/Input/Audio")
                                PW.Manager.connectStreamInput(root.id);
                        } else if (checked === false || root.isBlocklisted) {
                            PW.Manager.disconnectStream(root.id);
                        }
                    }
                }

                Controls.CheckBox {
                    text: i18n("Exclude") // qmllint disable
                    checked: root.isBlocklisted
                    Controls.ToolTip.text: i18n("Add %1 to excluded applications", root.name)
                    Controls.ToolTip.visible: hovered
                    onCheckedChanged: {
                        if (root.model.isBlocklisted !== checked)
                            root.model.isBlocklisted = checked;
                    }
                }
            }

            RowLayout {
                Layout.columnSpan: delegateLayout.columns

                Controls.Button {
                    id: muteButton

                    icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-high-symbolic"
                    checkable: true
                    checked: root.mute
                    onCheckedChanged: {
                        if (checked !== root.mute)
                            PW.Manager.setNodeMute(root.serial, checked);
                    }
                }

                Controls.Slider {
                    id: volumeSlider

                    function prepareVolumeValue(normalizedValue) {
                        const v = DbMain.useCubicVolumes === false ? normalizedValue : Math.cbrt(normalizedValue);
                        return v * 100;
                    }

                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                    value: prepareVolumeValue(root.volume)
                    to: 100
                    stepSize: 1
                    enabled: !muteButton.checked
                    wheelEnabled: false
                    onMoved: {
                        if (value !== prepareVolumeValue(root.volume)) {
                            let v = value / 100;
                            v = DbMain.useCubicVolumes === false ? v : v * v * v;
                            PW.Manager.setNodeVolume(root.serial, root.nVolumeChannels, v);
                        }
                    }
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: `${Math.round(volumeSlider.value)} ${Units.percent}`
                }
            }
        }
    }
}
