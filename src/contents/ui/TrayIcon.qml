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

import Qt.labs.platform
import QtQuick
import ee.pipewire as PW
import ee.presets as Presets
import org.kde.kirigami as Kirigami

SystemTrayIcon {
    id: tray

    required property string applicationName
    required property string applicationId
    required property bool canUseSysTray
    required property ShortcutsSheet shortcuts
    property Main mainWindow: null

    visible: DbMain.showTrayIcon && canUseSysTray
    icon.name: applicationId + "-symbolic"
    tooltip: applicationName

    onActivated: {
        if (mainWindow && !mainWindow.visible) {
            mainWindow.show();
            mainWindow.raise();
            mainWindow.requestActivate();
        } else if (mainWindow) {
            mainWindow.hide();
        }
    }

    function setMainWindow(window) {
        mainWindow = window;
    }

    menu: Menu {
        id: trayMenu
        visible: false
        onAboutToShow: {
            instantiatorInputPresets.model = [];
            instantiatorOutputPresets.model = [];

            instantiatorInputPresets.model = DbStreamInputs.mostUsedPresets;
            instantiatorOutputPresets.model = DbStreamOutputs.mostUsedPresets;

            /**
                 * Although it is possible to make a binding to the text property so it is automatically updated,
                 * it is possible that the menu is constructed before the description is available for the node name. In
                 * this situation we can have an empty string coming from getNodeDescription, and it will stay empty
                 * until something forces the database device name property to be changed. It is more reliable
                 * to read the description when the user opens the tray icon menu.
                 */

            inputDeviceMenuItem.text = PW.ModelNodes.getNodeDescription(DbStreamInputs.inputDevice);
            outputDeviceMenuItem.text = PW.ModelNodes.getNodeDescription(DbStreamOutputs.outputDevice);
        }

        Instantiator {
            id: instantiatorInputPresets

            delegate: MenuItem {
                text: modelData // qmllint disable
                checkable: true
                checked: DbMain.lastLoadedInputPreset === modelData
                onTriggered: {
                    Presets.Manager.loadLocalPresetFile(0, modelData); // qmllint disable

                    tray.showMessage(i18n("Preset Loaded"), modelData, SystemTrayIcon.Information, 3000); // qmllint disable
                }
            }

            onObjectAdded: (index, object) => trayMenu.insertItem(trayMenu.items.indexOf(sectionInputPresets), object)
            onObjectRemoved: (index, object) => trayMenu.removeItem(object)
        }

        Instantiator {
            id: instantiatorOutputPresets

            delegate: MenuItem {
                text: modelData // qmllint disable
                checkable: true
                checked: DbMain.lastLoadedOutputPreset === modelData
                onTriggered: {
                    Presets.Manager.loadLocalPresetFile(1, modelData); // qmllint disable

                    tray.showMessage(i18n("Preset Loaded"), modelData, SystemTrayIcon.Information, 3000); // qmllint disable
                }
            }

            onObjectAdded: (index, object) => trayMenu.insertItem(trayMenu.items.indexOf(sectionOutputPresets), object)
            onObjectRemoved: (index, object) => trayMenu.removeItem(object)
        }

        Kirigami.PromptDialog {
            id: clearMostUsedOutputDialog

            title: i18n("Clear List") // qmllint disable
            subtitle: i18n("Are you sure you want to clear this list?") // qmllint disable
            standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
            onAccepted: {
                DbStreamOutputs.usedPresets = [];
                DbStreamOutputs.mostUsedPresets = [];
            }
        }

        Kirigami.PromptDialog {
            id: clearMostUsedInputDialog

            title: i18n("Clear List") // qmllint disable
            subtitle: i18n("Are you sure you want to clear this list?") // qmllint disable
            standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
            onAccepted: {
                DbStreamInputs.usedPresets = [];
                DbStreamInputs.mostUsedPresets = [];
            }
        }

        MenuItem {
            text: i18n("Input Presets") // qmllint disable
            icon.name: "user-bookmarks-symbolic"
            onTriggered: {
                mainWindow.show();
                mainWindow.raise();

                clearMostUsedInputDialog.open();
            }
        }

        MenuSeparator {
            id: sectionInputPresets
        }

        MenuItem {
            text: i18n("Output Presets") // qmllint disable
            icon.name: "user-bookmarks-symbolic"
            onTriggered: {
                mainWindow.show();
                mainWindow.raise();

                clearMostUsedOutputDialog.open();
            }
        }

        MenuSeparator {
            id: sectionOutputPresets
        }

        MenuItem {
            id: inputDeviceMenuItem

            icon.name: "audio-input-microphone-symbolic"
            enabled: false
        }

        MenuItem {
            id: outputDeviceMenuItem

            icon.name: "audio-speakers-symbolic"
            enabled: false
        }

        MenuSeparator {}

        MenuItem {
            text: i18n("Active") // qmllint disable
            checkable: true
            checked: !DbMain.bypass
            onTriggered: {
                DbMain.bypass = !checked;
            }
        }

        MenuSeparator {}

        MenuItem {
            text: i18n("Shortcuts") // qmllint disable
            icon.name: "input-keyboard-symbolic"
            onTriggered: {
                mainWindow.show();
                shortcuts.open(); // shortcutsSheet is created in Main.qml
            }
        }

        MenuItem {
            text: i18n("Manual") // qmllint disable
            icon.name: "help-contents-symbolic"
            onTriggered: {
                Qt.openUrlExternally("https://wwmm.github.io/easyeffects/");
            }
        }

        MenuSeparator {}

        MenuItem {
            text: i18n("Quit") // qmllint disable
            icon.name: "application-exit-symbolic"
            onTriggered: Qt.quit()
        }
    }
}
