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
import ee.presets as Presets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: columnLayout

    spacing: 0

    GridLayout {
        columns: 2
        rowSpacing: 0
        columnSpacing: 0

        FormCard.FormComboBoxDelegate {
            id: device

            readonly property int deviceId: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.DeviceId);
            }
            readonly property string deviceName: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.Name);
            }

            property string deviceRouteDescription

            Layout.margins: Kirigami.Units.smallSpacing
            verticalPadding: 0
            text: i18n("Device") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "description"
            editable: false
            model: DbMain.visiblePage === 0 ? PW.ModelSinkDevices : PW.ModelSourceDevices
            description: `${i18n("Route")}: ${deviceRouteDescription}` // qmllint disable

            function updateRouteDescription() {
                const proxyIndex = model.index(currentIndex, 0);

                return model.data(proxyIndex, PW.ModelNodes.DeviceRouteDescription);
            }

            onCurrentIndexChanged: {
                device.deviceRouteDescription = device.updateRouteDescription();
            }

            Connections {
                target: device.model

                function onModelReset() {
                    device.currentIndex = 0;

                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onRowsInserted() {
                    device.currentIndex = 0;

                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onRowsRemoved() {
                    device.currentIndex = 0;

                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onDataChanged() {
                    device.currentIndex = 0;

                    device.deviceRouteDescription = device.updateRouteDescription();
                }
            }
        }

        Kirigami.Icon {
            source: DbMain.visiblePage === 1 ? "audio-input-microphone-symbolic" : "audio-speakers-symbolic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            Layout.margins: Kirigami.Units.smallSpacing
        }

        FormCard.FormComboBoxDelegate {
            id: preset

            Layout.margins: Kirigami.Units.smallSpacing
            verticalPadding: 0
            text: i18n("Local preset") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "name"
            editable: false
            model: DbMain.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
        }

        Controls.Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            Layout.margins: Kirigami.Units.smallSpacing
            text: i18n("Create") // qmllint disable
            icon.name: "list-add-symbolic"
            Controls.ToolTip.text: i18n("Create a new autoload preset")
            Controls.ToolTip.visible: hovered
            onClicked: {
                const deviceId = device.deviceId;
                const deviceName = device.deviceName;
                const deviceDescription = device.currentText;
                const presetName = preset.currentText;
                const deviceRoute = device.deviceRouteDescription;

                if (Common.isEmpty(presetName)) {
                    return;
                }

                if (DbMain.visiblePage === 0)
                    Presets.Manager.addAutoload(1, presetName, deviceName, deviceDescription, deviceRoute);
                else if (DbMain.visiblePage === 1)
                    Presets.Manager.addAutoload(0, presetName, deviceName, deviceDescription, deviceRoute);
            }
        }
    }

    RowLayout {
        id: listviewRow

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            reuseItems: true
            model: DbMain.visiblePage === 0 ? Presets.SortedAutoloadOutputListModel : Presets.SortedAutoloadInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty List") // qmllint disable
                icon.name: "notification-empty"
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string deviceName
                required property string deviceDescription
                required property string deviceProfile
                required property string devicePreset

                width: ListView.view.width

                background: Kirigami.FlexColumn {
                    maximumWidth: Kirigami.Units.gridUnit * 40

                    Kirigami.Separator {
                        Layout.alignment: Qt.AlignBottom
                        Layout.fillWidth: true
                        visible: listItemDelegate.index !== 0
                    }
                }

                contentItem: Kirigami.FlexColumn {

                    GridLayout {
                        id: delegateLayout

                        columns: 3
                        rows: 4

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Device") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            text: listItemDelegate.deviceName
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Button {
                            Layout.alignment: Qt.AlignCenter | Qt.AlignRight
                            Layout.rowSpan: 4
                            icon.name: "delete"
                            Controls.ToolTip.text: i18n("Remove this autoload preset")
                            Controls.ToolTip.visible: hovered
                            onClicked: {
                                if (DbMain.visiblePage === 0)
                                    Presets.Manager.removeAutoload(1, listItemDelegate.devicePreset, listItemDelegate.deviceName, listItemDelegate.deviceProfile);
                                else if (DbMain.visiblePage === 1)
                                    Presets.Manager.removeAutoload(0, listItemDelegate.devicePreset, listItemDelegate.deviceName, listItemDelegate.deviceProfile);
                            }
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Description") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            text: listItemDelegate.deviceDescription
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Route") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            text: listItemDelegate.deviceProfile
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            text: i18n("Local preset") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.NoWrap
                            elide: Text.ElideRight
                            text: listItemDelegate.devicePreset
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
            }
        }

        Controls.ScrollBar {
            id: listViewScrollBar

            parent: listviewRow
            Layout.fillHeight: true
        }
    }
}
