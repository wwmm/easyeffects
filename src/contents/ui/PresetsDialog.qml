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
import ee.presets as Presets
import ee.ui
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Dialog {
    id: control

    readonly property string lastLoadedPresetName: {
        if (DbMain.visiblePage === 0)
            return DbMain.lastLoadedOutputPreset;
        else if (DbMain.visiblePage === 1)
            return DbMain.lastLoadedInputPreset;
        return "";
    }
    readonly property string lastLoadedCommunityPackage: {
        if (DbMain.visiblePage === 0)
            return DbMain.lastLoadedOutputCommunityPackage;
        else if (DbMain.visiblePage === 1)
            return DbMain.lastLoadedInputCommunityPackage;
        return "";
    }

    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8) // qmllint disable
    implicitHeight: Math.min(Kirigami.Units.gridUnit * 40, Math.round(Controls.ApplicationWindow.window.height * 0.8))
    bottomPadding: 1
    anchors.centerIn: parent

    onAboutToShow: {
        pageLoader.active = true;
    }

    onAboutToHide: {
        pageLoader.active = false;
    }

    Loader {
        id: pageLoader

        height: control.height - control.header.height - control.footer.height
        active: false

        source: {
            switch (DbMain.visiblePresetSheetPage) {
            case 0:
                return Qt.resolvedUrl("PresetsLocalPage.qml");
            case 1:
                return Qt.resolvedUrl("PresetsCommunityPage.qml");
            case 2:
                return Qt.resolvedUrl("PresetsAutoloadPage.qml");
            default:
                return "";
            }
        }
    }

    header: Item {
        width: parent.width
        height: segmentedButton.height + 2 * Kirigami.Units.smallSpacing

        Components.SegmentedButton {
            id: segmentedButton

            anchors {
                centerIn: parent
                margins: Kirigami.Units.smallSpacing
            }

            readonly property bool hasEnoughWidth: appWindow.width >= Kirigami.Units.gridUnit * 40

            readonly property var displayHint: (!Kirigami.Settings.isMobile && hasEnoughWidth) ? Kirigami.DisplayHint.KeepVisible : Kirigami.DisplayHint.IconOnly

            actions: [
                Kirigami.Action {
                    text: i18n("Local")
                    icon.name: "system-file-manager-symbolic"
                    checkable: true
                    checked: DbMain.visiblePresetSheetPage === 0
                    displayHint: segmentedButton.displayHint
                    onTriggered: DbMain.visiblePresetSheetPage = 0
                },
                Kirigami.Action {
                    text: i18n("Community")
                    icon.name: "system-users-symbolic"
                    checkable: true
                    checked: DbMain.visiblePresetSheetPage === 1
                    displayHint: segmentedButton.displayHint
                    onTriggered: DbMain.visiblePresetSheetPage = 1
                },
                Kirigami.Action {
                    text: i18n("Autoload")
                    icon.name: "task-recurring-symbolic"
                    checkable: true
                    checked: DbMain.visiblePresetSheetPage === 2
                    displayHint: segmentedButton.displayHint
                    onTriggered: DbMain.visiblePresetSheetPage = 2
                }
            ]
        }

        Controls.ToolButton {
            text: i18nc("@action:button", "Close")
            icon.name: 'dialog-close-symbolic'
            display: Controls.ToolButton.IconOnly
            onClicked: control.close()
            anchors {
                margins: Kirigami.Units.smallSpacing
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
        }
    }

    footer: Loader {
        Layout.fillWidth: true
        Layout.fillHeight: false
        sourceComponent: DbMain.visiblePresetSheetPage === 2 ? fallbackPresetComponent : messageComponent

        Component {
            id: messageComponent

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                Layout.maximumWidth: parent.width
                position: Kirigami.InlineMessage.Position.Footer
                visible: DbMain.visiblePresetSheetPage !== 2
                text: {
                    if (Common.isEmpty(control.lastLoadedPresetName))
                        return i18n("No preset loaded"); // qmllint disable

                    const presetType = Common.isEmpty(lastLoadedCommunityPackage) ? i18n("Local") : i18n("Community"); // qmllint disable

                    return `${presetType}: <strong>${control.lastLoadedPresetName}<strong>`;
                }
            }
        }

        Component {
            id: fallbackPresetComponent

            RowLayout {

                FormCard.FormComboBoxDelegate {
                    id: fallbackPreset

                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.fillWidth: true
                    verticalPadding: 0
                    text: i18n("Fallback Preset") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: {
                        const fallbackPreset = DbMain.visiblePage === 0 ? DbMain.outputAutoloadingFallbackPreset : DbMain.inputAutoloadingFallbackPreset;
                        for (let n = 0; n < model.rowCount(); n++) {
                            const proxyIndex = model.index(n, 0);
                            const name = model.data(proxyIndex, PresetsListModel.Name);
                            if (name === fallbackPreset)
                                return n;
                        }
                        return 0;
                    }
                    textRole: "name"
                    editable: false
                    enabled: DbMain.visiblePage === 0 ? DbMain.outputAutoloadingUsesFallback : DbMain.inputAutoloadingUsesFallback
                    model: DbMain.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
                    onActivated: idx => {
                        if (DbMain.visiblePage === 0)
                            DbMain.outputAutoloadingFallbackPreset = currentText;
                        else if (DbMain.visiblePage === 1)
                            DbMain.inputAutoloadingFallbackPreset = currentText;
                    }
                }

                EeSwitch {
                    Layout.fillWidth: false
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                    Layout.margins: Kirigami.Units.smallSpacing
                    isChecked: DbMain.visiblePage === 0 ? DbMain.outputAutoloadingUsesFallback : DbMain.inputAutoloadingUsesFallback
                    verticalPadding: 0
                    onCheckedChanged: {
                        if (DbMain.visiblePage === 0) {
                            if (isChecked !== DbMain.outputAutoloadingUsesFallback)
                                DbMain.outputAutoloadingUsesFallback = isChecked;
                        } else if (DbMain.visiblePage === 1) {
                            if (isChecked !== DbMain.inputAutoloadingUsesFallback)
                                DbMain.inputAutoloadingUsesFallback = isChecked;
                        }
                    }
                }
            }
        }
    }
}
