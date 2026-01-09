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
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.autostart //qmllint disable
import ee.database as DB
import ee.pipeline as Pipeline
import ee.pipewire as PW
import ee.presets as Presets
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: appWindow

    // We need to set visible to false in order to fix an issue related to
    // --hide-window option. See #4491.
    visible: false
    width: DbMain.width
    height: DbMain.height
    title: applicationName
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    readonly property real maxOverlayHeight: height - header.height - footer.height

    property var pagesMap: {
        0: {
            page: Qt.resolvedUrl("./PageStreamsEffects.qml"),
            pageType: 0,
            streamDB: DbStreamOutputs,
            pipelineInstance: Pipeline.Output
        },
        1: {
            page: Qt.resolvedUrl("./PageStreamsEffects.qml"),
            pageType: 1,
            streamDB: DbStreamInputs,
            pipelineInstance: Pipeline.Input
        },
        2: {
            page: Qt.resolvedUrl("./PipeWirePage.qml"),
            pageType: 2,
            streamDB: null,
            pluginsDB: null,
            pipelineInstance: null
        }
    }

    onVisibleChanged: {
        if (appWindow.visible) {
            /**
             * When the window is reopened, its state (maximized, minimized,
             * fullscreen, etc.) is lost.
             * This happens both after restarting the application and when
             * switching its visibility through the tray.
             * Until this issue is fixed with a proper Wayland protocol, we can
             * adopt the approach used in qBittorrent: save the visibility
             * property into the database when hiding the window and restore
             * its value when the window is shown.
             */

            /**
             * The idea was doing what is described above. But for some reason Qt sometimes shows a warning about
             * visible conflicting with visibility. And in the worse cases the window may never be shown. It does not
             * matter how many times we restart EE. So for now letś disable the visibility management.
             */

            // appWindow.visibility = DbMain.visibility;

            DB.Manager.enableAutosave(true);

            openMappedPage(DbMain.visiblePage);
        } else {
            // DbMain.visibility = appWindow.visibility;

            DB.Manager.saveAll();

            pageStack.clear();
        }
    }

    onClosing: {
        onCloseWindow();
    }

    Component.onDestruction: {
        onCloseWindow();
    }

    function onCloseWindow() {
        // DbMain.visibility = appWindow.visibility;

        DB.Manager.saveAll();

        DB.Manager.enableAutosave(false);
    }

    function openMappedPage(index) {
        const info = pagesMap[index];

        if (!info)
            return;

        let args = {};

        if (info.pageType !== 2) {
            args.pageType = info.pageType;
            args.streamDB = info.streamDB;

            args.pluginsDB = index === 0 ? Qt.binding(function () {
                // QMap used as property is viewed as a JS object and not a QObject. So binding needs Qt.binding
                return DB.Manager.soePluginsDB;
            }) : Qt.binding(function () {
                return DB.Manager.siePluginsDB;
            });

            args.pipelineInstance = info.pipelineInstance;
            args.visible = true;
        }

        if (pageStack.depth === 0)
            pageStack.push(info.page, args);
        else
            pageStack.replace(info.page, args);

        DbMain.visiblePage = index;
    }

    function showStatus(label, statusType = Kirigami.MessageType.Information, autohide = true) {
        autoHideStatusTimer.stop();

        status.type = statusType;
        status.text = label;
        status.visible = true;

        if (autohide) {
            autoHideStatusTimer.start();
        }
    }

    Binding {
        target: DbMain
        property: "width"
        value: appWindow.width
    }

    Binding {
        target: DbMain
        property: "height"
        value: appWindow.height
    }

    Connections {
        target: DbMain

        function onVisiblePageChanged() {
            appWindow.openMappedPage(DbMain.visiblePage);
        }
    }

    Connections {
        target: Presets.Manager

        function onPresetLoadError(title, description) {
            appWindow.showStatus(`${title}. ${description}.`, Kirigami.MessageType.Error, false);
        }
    }

    Connections {
        target: Autostart

        function onError(description) {
            appWindow.showStatus(`${description}`, Kirigami.MessageType.Error, false);
        }
    }

    Timer {
        id: autoHideStatusTimer
        interval: DbMain.autoHideInlineMessageTimeout
        onTriggered: {
            status.visible = false;

            autoHideStatusTimer.stop();
        }
    }

    Shortcut {
        sequences: ["F11"]
        onActivated: {
            if (appWindow.visibility !== Window.FullScreen)
                appWindow.showFullScreen();
            else
                appWindow.showNormal();
        }
    }

    Shortcut {
        sequences: ["F1"]
        onActivated: {
            Qt.openUrlExternally("https://wwmm.github.io/easyeffects/");
        }
    }

    Shortcut {
        sequences: ["Ctrl+B"]
        onActivated: {
            DbMain.bypass = !DbMain.bypass;
        }
    }

    Shortcut {
        sequences: ["Ctrl+W"]
        onActivated: appWindow.close()
    }

    Shortcut {
        sequences: ["Ctrl+Q"]
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: ["Ctrl+Shift+I"]
        onActivated: {
            DbMain.processAllInputs = !DbMain.processAllInputs;
        }
    }

    Shortcut {
        sequences: ["Ctrl+Shift+O"]
        onActivated: {
            DbMain.processAllOutputs = !DbMain.processAllOutputs;
        }
    }

    Kirigami.Action {
        /**
         * The usual way to declare the "shut down the service" Ctrl+Q shortcut
         * with the Shortcut QML class does not work on Gnome since it closes
         * only the visible window and does not quit the Easy Effects service.
         * With a Kirigami.Action class instead we can implement a
         * StandardKey.Quit action that catches the CTRL+Q event on Gnome, so
         * we can terminate the service with the keyboard shortcut also there.
         */
        shortcut: StandardKey.Quit
        onTriggered: {
            Qt.quit();
        }
    }

    PreferencesSheet {
        id: preferencesSheet
        window: appWindow
    }

    PresetsDialog {
        id: presetsDialog
    }

    ShortcutsSheet {
        id: shortcutsSheet
    }

    Kirigami.PromptDialog {
        id: resetPromptDialog

        title: i18n("Reset Settings?") // qmllint disable
        subtitle: i18n("Are you sure you want to reset all Easy Effects settings?") // qmllint disable
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: DB.Manager.resetAll()
    }

    SystemTrayIcon {
        id: tray

        visible: DbMain.showTrayIcon && canUseSysTray // qmllint disable
        icon.name: applicationId + "-symbolic"
        tooltip: applicationName
        onActivated: {
            if (!appWindow.visible) {
                appWindow.show();
                appWindow.raise();
                appWindow.requestActivate();
            } else {
                appWindow.hide();
            }
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
                    appWindow.show();
                    appWindow.raise();

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
                    appWindow.show();
                    appWindow.raise();

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
                    appWindow.show();
                    shortcutsSheet.open();
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

    header: Kirigami.AbstractApplicationHeader {

        contentItem: RowLayout {
            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.smallSpacing
                right: parent.right
                rightMargin: Kirigami.Units.smallSpacing
            }

            Kirigami.ActionToolBar {
                id: presetsToolBar

                alignment: Qt.AlignLeft
                overflowIconName: "overflow-menu-left"
                actions: [
                    Kirigami.Action {
                        text: i18n("Go back") // qmllint disable
                        icon.name: "draw-arrow-back-symbolic"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        // Use nullish coalescing to avoid the `Unable to assign [undefined] to bool` message.
                        visible: (pageStack.currentItem as Kirigami.Page)?.showBackButton ?? false
                        onTriggered: {
                            pageStack.currentItem.goBack();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Effects") // qmllint disable
                        tooltip: i18n("Turn effects on/off") // qmllint disable
                        icon.name: "system-shutdown-symbolic"
                        icon.color: checked === true ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        checkable: true
                        checked: !DbMain.bypass
                        onTriggered: {
                            if (checked !== !DbMain.bypass) {
                                DbMain.bypass = !checked;
                            }

                            checked ? appWindow.showStatus(i18n("Audio effects are enabled."), Kirigami.MessageType.Positive) : appWindow.showStatus(i18n("Audio effects are disabled.")); // qmllint disable
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Presets") // qmllint disable
                        icon.name: "user-bookmarks-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: DbMain.visiblePage !== 2
                        onTriggered: {
                            presetsDialog.open();
                        }
                    }
                ]
            }

            RowLayout {
                id: segmentedButton

                readonly property bool hasEnoughWidth: appWindow.width >= Kirigami.Units.gridUnit * 40
                readonly property int display: (!Kirigami.Settings.isMobile && hasEnoughWidth) ? Controls.ToolButton.TextBesideIcon : Controls.ToolButton.IconOnly

                spacing: Kirigami.Units.smallSpacing

                Layout.alignment: Qt.AlignHCenter

                Controls.ButtonGroup {
                    id: navButtonGroup
                }

                Controls.ToolButton {
                    text: i18n("Output") // qmllint disable
                    icon.name: "audio-speakers-symbolic"
                    checkable: true
                    checked: DbMain.visiblePage === 0
                    display: segmentedButton.display
                    onClicked: {
                        DbMain.visiblePage = 0;
                    }

                    Controls.ButtonGroup.group: navButtonGroup
                }

                Controls.ToolButton {
                    text: i18n("Input") // qmllint disable
                    icon.name: "audio-input-microphone-symbolic"
                    checkable: true
                    checked: DbMain.visiblePage === 1
                    display: segmentedButton.display
                    onClicked: {
                        DbMain.visiblePage = 1;
                    }

                    Controls.ButtonGroup.group: navButtonGroup
                }

                Controls.ToolButton {
                    text: i18n("PipeWire") // qmllint disable
                    icon.name: "network-server-symbolic"
                    checkable: true
                    checked: DbMain.visiblePage === 2
                    display: segmentedButton.display
                    onClicked: {
                        DbMain.visiblePage = 2;
                    }

                    Controls.ButtonGroup.group: navButtonGroup
                }
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                overflowIconName: "application-menu-symbolic"
                actions: [
                    Kirigami.Action {
                        text: i18n("Preferences") // qmllint disable
                        icon.name: "gtk-preferences-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            preferencesSheet.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Shortcuts") // qmllint disable
                        icon.name: "input-keyboard-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            shortcutsSheet.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Manual") // qmllint disable
                        icon.name: "help-contents-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            Qt.openUrlExternally("https://wwmm.github.io/easyeffects/");
                        }
                    },
                    Kirigami.Action {
                        separator: true
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                    },
                    Kirigami.Action {
                        text: i18n("Reset") // qmllint disable
                        icon.name: "edit-reset-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            resetPromptDialog.open();
                        }
                    },
                    Kirigami.Action {
                        separator: true
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                    },
                    Kirigami.Action {
                        text: i18n("About Easy Effects") // qmllint disable
                        icon.name: applicationId + "-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            appWindow.pageStack.pushDialogLayer(Qt.resolvedUrl("./AboutPage.qml"));
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Quit") // qmllint disable
                        icon.name: "application-exit-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            Qt.quit();
                        }
                    }
                ]
            }
        }
    }

    footer: Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        visible: false
        showCloseButton: true
    }
}
