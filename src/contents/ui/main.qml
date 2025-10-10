import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.help as Help
import ee.pipeline as Pipeline
import ee.presets as Presets
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: appWindow

    width: DB.Manager.main.width
    height: DB.Manager.main.height
    title: "Easy Effects"
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None

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

    onVisibleChanged: {
        if (appWindow.visible) {
            DB.Manager.enableAutosave(true);
        } else {
            DB.Manager.saveAll();
        }
    }

    onClosing: {
        console.log("main window is closing");

        DB.Manager.enableAutosave(false);

        gc();
    }

    Component.onCompleted: {
        openMappedPage(DB.Manager.main.visiblePage);
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

    PreferencesSheet {
        id: preferencesSheet
    }

    PresetsSheet {
        id: presetsSheet
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

    Kirigami.OverlaySheet {
        id: aboutSheet

        parent: applicationWindow().overlay// qmllint disable
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
        focus: true
        y: appWindow.header.height + Kirigami.Units.gridUnit

        Kirigami.AboutPage {
            id: aboutPage

            implicitWidth: Kirigami.Units.gridUnit * 30
            aboutData: {
                "displayName": "Easy Effects",
                "productName": "easyeffects",
                "componentName": "easyeffects",
                "shortDescription": i18n("Audio effects for PipeWire applications") // qmllint disable
                ,
                "homepage": "https://github.com/wwmm/easyeffects",
                "bugAddress": "https://github.com/wwmm/easyeffects/issues",
                "donateUrl": "https://github.com/wwmm/easyeffects",
                "getInvolvedUrl": "https://github.com/wwmm/easyeffects",
                "version": projectVersion// qmllint disable
                ,
                "programLogo": "com.github.wwmm.easyeffects",
                "otherText": "",
                "authors": [
                    {
                        "name": "Wellington Wallace",
                        "task": i18nc("@info:credit", "Developer") // qmllint disable
                        ,
                        "emailAddress": "wellingtonwallace@gmail.com",
                        "webAddress": "",
                        "ocsUsername": ""
                    }
                ],
                "credits": [],
                "translators": [],
                "licenses": [
                    {
                        "name": "GPL v3",
                        "text": "https://github.com/wwmm/easyeffects/blob/master/LICENSE",
                        "spdx": "GPL-3.0"
                    }
                ],
                "copyrightStatement": "© 2017-2025 EasyEffects Team",
                "desktopFileName": "com.github.wwmm.easyeffects.desktop"
            }
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

            Instantiator {
                id: instantiatorInputPresets

                model: DB.Manager.streamInputs.mostUsedPresets
                delegate: MenuItem {
                    text: modelData// qmllint disable
                    onTriggered: {
                        Presets.Manager.loadLocalPresetFile(0, modelData);// qmllint disable
                    }
                }

                onObjectAdded: (index, object) => trayMenu.insertItem(trayMenu.items.indexOf(sectionInputPresets), object)
                onObjectRemoved: (index, object) => trayMenu.removeItem(object)
            }

            Instantiator {
                id: instantiatorOutputPresets

                model: DB.Manager.streamOutputs.mostUsedPresets
                delegate: MenuItem {
                    text: modelData// qmllint disable
                    onTriggered: {
                        Presets.Manager.loadLocalPresetFile(1, modelData);// qmllint disable
                    }
                }

                onObjectAdded: (index, object) => trayMenu.insertItem(trayMenu.items.indexOf(sectionOutputPresets), object)
                onObjectRemoved: (index, object) => trayMenu.removeItem(object)
            }

            MenuItem {
                text: i18n("Input Presets") // qmllint disable
                icon.name: "bookmarks-symbolic"
                enabled: false
            }

            MenuSeparator {
                id: sectionInputPresets
            }

            MenuItem {
                text: i18n("Output Presets") // qmllint disable
                icon.name: "bookmarks-symbolic"
                enabled: false
            }

            MenuSeparator {
                id: sectionOutputPresets
            }

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
                    Help.Manager.openManual();
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
                alignment: Qt.AlignLeft
                overflowIconName: "overflow-menu-left"
                actions: [
                    Kirigami.Action {
                        text: i18n("Presets") // qmllint disable
                        icon.name: "bookmarks-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        onTriggered: {
                            presetsSheet.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Turn Effects On/Off") // qmllint disable
                        icon.name: "system-shutdown-symbolic"
                        icon.color: checked === true ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.negativeTextColor
                        displayHint: Kirigami.DisplayHint.IconOnly
                        checkable: true
                        checked: !DB.Manager.main.bypass
                        onTriggered: {
                            if (checked !== !DB.Manager.main.bypass)
                                DB.Manager.main.bypass = !checked;
                        }
                    }
                ]
            }

            Kirigami.ActionToolBar {
                id: tabbar

                alignment: Qt.AlignHCenter
                actions: [
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "audio-speakers-symbolic"
                        text: i18n("Output") // qmllint disable
                        checkable: true
                        checked: DB.Manager.main.visiblePage === 0
                        onTriggered: {
                            DB.Manager.main.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "audio-input-microphone-symbolic"
                        text: i18n("Input") // qmllint disable
                        checkable: true
                        checked: DB.Manager.main.visiblePage === 1
                        onTriggered: {
                            DB.Manager.main.visiblePage = 1;
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "network-server-symbolic"
                        text: i18n("PipeWire") // qmllint disable
                        checkable: true
                        checked: DB.Manager.main.visiblePage === 2
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
                            Help.Manager.openManual();
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
}
