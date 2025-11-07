import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipeline as Pipeline
import ee.pipewire as PW
import ee.presets as Presets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components

Kirigami.ApplicationWindow {
    id: appWindow

    width: DB.Manager.main.width
    height: DB.Manager.main.height
    title: "Easy Effects"
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

    readonly property real maxOverlayHeight: height - header.height - footer.height

    property var pagesMap: {
        0: {
            page: "qrc:/ui/PageStreamsEffects.qml",
            pageType: 0,
            streamDB: DB.Manager.streamOutputs,
            pipelineInstance: Pipeline.Output
        },
        1: {
            page: "qrc:/ui/PageStreamsEffects.qml",
            pageType: 1,
            streamDB: DB.Manager.streamInputs,
            pipelineInstance: Pipeline.Input
        },
        2: {
            page: "qrc:/ui/PipeWirePage.qml",
            pageType: 2,
            streamDB: null,
            pluginsDB: null,
            pipelineInstance: null
        }
    }

    onVisibleChanged: {
        if (appWindow.visible) {
            DB.Manager.enableAutosave(true);

            openMappedPage(DB.Manager.main.visiblePage);
        } else {
            DB.Manager.saveAll();

            pageStack.clear();
        }
    }

    onClosing: {
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

        DB.Manager.main.visiblePage = index;
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
        target: DB.Manager.main
        property: "width"
        value: appWindow.width
    }

    Binding {
        target: DB.Manager.main
        property: "height"
        value: appWindow.height
    }

    Connections {
        target: DB.Manager.main

        function onVisiblePageChanged() {
            appWindow.openMappedPage(DB.Manager.main.visiblePage);
        }
    }

    Connections {
        target: Presets.Manager

        function onPresetLoadError(title, description) {
            appWindow.showStatus(`${title}. ${description}.`, Kirigami.MessageType.Error, false);
        }
    }

    Timer {
        id: autoHideStatusTimer
        interval: DB.Manager.main.autoHideInlineMessageTimeout
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
            Help.Manager.openManual();
        }
    }

    Shortcut {
        sequences: ["Ctrl+B"]
        onActivated: {
            DB.Manager.main.bypass = !DB.Manager.main.bypass;
        }
    }

    Shortcut {
        sequences: ["Ctrl+W"]
        onActivated: appWindow.close()
    }

    Shortcut {
        // This one replaces CTRL+Q that might not work outside KDE.
        sequences: ["Ctrl+T"]
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: ["Ctrl+Shift+I"]
        onActivated: {
            DB.Manager.main.processAllInputs = !DB.Manager.main.processAllInputs;
        }
    }

    Shortcut {
        sequences: ["Ctrl+Shift+O"]
        onActivated: {
            DB.Manager.main.processAllOutputs = !DB.Manager.main.processAllOutputs;
        }
    }

    PreferencesSheet {
        id: preferencesSheet
    }

    PresetsSheet {
        id: presetsSheet
    }

    ShortcutsSheet {
        id: shortcutsSheet
    }

    HelpSheet {
        id: helpSheet
    }

    Kirigami.PromptDialog {
        id: resetPromptDialog

        title: i18n("Reset Settings?") // qmllint disable
        subtitle: i18n("Are you sure you want to reset all Easy Effects settings?") // qmllint disable
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: DB.Manager.resetAll()
    }

    Kirigami.OverlaySheet {
        id: aboutSheet

        parent: applicationWindow().overlay// qmllint disable
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
        focus: true
        y: 0

        implicitWidth: Kirigami.Units.gridUnit * 30

        Loader {
            id: aboutPageLoader

            height: appWindow.maxOverlayHeight
            source: "qrc:/ui/AboutPage.qml"
        }
    }

    SystemTrayIcon {
        id: tray

        visible: DB.Manager.main.showTrayIcon && canUseSysTray // qmllint disable
        icon.name: "com.github.wwmm.easyeffects"
        tooltip: "Easy Effects"
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

                instantiatorInputPresets.model = DB.Manager.streamInputs.mostUsedPresets;
                instantiatorOutputPresets.model = DB.Manager.streamOutputs.mostUsedPresets;

                /**
                 * Although it is possible to make a binding to the text property so it is automatically updated
                 * it is possible that the menu is contructed before description is available for the node name. In
                 * this situation we can have an empty string coming from getNodeDescription. And it will stay empty
                 * until something forces the database device name property to be changed. it is more reliable
                 * to read the description when the user opens the tray icon menu.
                 */

                inputDeviceMenuItem.text = PW.ModelNodes.getNodeDescription(DB.Manager.streamInputs.inputDevice);
                outputDeviceMenuItem.text = PW.ModelNodes.getNodeDescription(DB.Manager.streamOutputs.outputDevice);
            }

            Instantiator {
                id: instantiatorInputPresets

                delegate: MenuItem {
                    text: modelData// qmllint disable
                    onTriggered: {
                        Presets.Manager.loadLocalPresetFile(0, modelData);// qmllint disable

                        tray.showMessage(i18n("Preset Loaded"), modelData, SystemTrayIcon.Information, 3000);// qmllint disable
                    }
                }

                onObjectAdded: (index, object) => trayMenu.insertItem(trayMenu.items.indexOf(sectionInputPresets), object)
                onObjectRemoved: (index, object) => trayMenu.removeItem(object)
            }

            Instantiator {
                id: instantiatorOutputPresets

                delegate: MenuItem {
                    text: modelData// qmllint disable
                    onTriggered: {
                        Presets.Manager.loadLocalPresetFile(1, modelData);// qmllint disable

                        tray.showMessage(i18n("Preset Loaded"), modelData, SystemTrayIcon.Information, 3000);// qmllint disable
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
                    DB.Manager.streamOutputs.usedPresets = [];
                    DB.Manager.streamOutputs.mostUsedPresets = [];
                }
            }

            Kirigami.PromptDialog {
                id: clearMostUsedInputDialog

                title: i18n("Clear List") // qmllint disable
                subtitle: i18n("Are you sure you want to clear this list?") // qmllint disable
                standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                onAccepted: {
                    DB.Manager.streamInputs.usedPresets = [];
                    DB.Manager.streamInputs.mostUsedPresets = [];
                }
            }

            MenuItem {
                text: i18n("Input Presets") // qmllint disable
                icon.name: "bookmarks-symbolic"
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
                icon.name: "bookmarks-symbolic"
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
                checked: !DB.Manager.main.bypass
                onTriggered: {
                    DB.Manager.main.bypass = !checked;
                }
            }

            MenuSeparator {}

            MenuItem {
                text: i18n("Shortcuts") // qmllint disable
                icon.name: "configure-shortcuts-symbolic"
                onTriggered: {
                    appWindow.show();
                    shortcutsSheet.open();
                }
            }

            MenuItem {
                text: i18n("Manual") // qmllint disable
                icon.name: "help-contents-symbolic"
                onTriggered: {
                    appWindow.show();
                    appWindow.raise();
                    helpSheet.open();
                }
            }

            MenuSeparator {}

            MenuItem {
                text: i18n("Quit") // qmllint disable
                icon.name: "gtk-quit"
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
                        text: i18n("Turn effects on/off") // qmllint disable
                        icon.name: "system-shutdown-symbolic"
                        icon.color: checked === true ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                        displayHint: Kirigami.DisplayHint.IconOnly
                        checkable: true
                        checked: !DB.Manager.main.bypass
                        onTriggered: {
                            if (checked !== !DB.Manager.main.bypass)
                                DB.Manager.main.bypass = !checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Presets") // qmllint disable
                        icon.name: "bookmarks-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: DB.Manager.main.visiblePage !== 2
                        onTriggered: {
                            presetsSheet.open();
                        }
                    }
                ]
            }

            Components.SegmentedButton {
                id: segmentedButton

                Layout.alignment: Qt.AlignHCenter

                readonly property bool hasEnoughWidth: appWindow.width >= Kirigami.Units.gridUnit * 40

                readonly property var displayHint: (!Kirigami.Settings.isMobile && hasEnoughWidth) ? Kirigami.DisplayHint.KeepVisible : Kirigami.DisplayHint.IconOnly

                actions: [
                    Kirigami.Action {
                        text: i18n("Output") // qmllint disable
                        icon.name: "audio-speakers-symbolic"
                        checked: DB.Manager.main.visiblePage === 0
                        displayHint: segmentedButton.displayHint
                        onTriggered: {
                            DB.Manager.main.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Input") // qmllint disable
                        icon.name: "audio-input-microphone-symbolic"
                        checked: DB.Manager.main.visiblePage === 1
                        displayHint: segmentedButton.displayHint
                        onTriggered: {
                            DB.Manager.main.visiblePage = 1;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("PipeWire") // qmllint disable
                        icon.name: "network-server-symbolic"
                        checked: DB.Manager.main.visiblePage === 2
                        displayHint: segmentedButton.displayHint
                        onTriggered: {
                            DB.Manager.main.visiblePage = 2;
                        }
                    }
                ]
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
                        icon.name: "configure-shortcuts-symbolic"
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
                            helpSheet.open();
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
                        icon.name: "com.github.wwmm.easyeffects"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            aboutSheet.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Quit") // qmllint disable
                        icon.name: "gtk-quit"
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
