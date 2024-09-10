import AboutEE
import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ApplicationWindow {
    id: appWindow

    width: EEdb.width
    height: EEdb.height
    title: i18nc("@title:window", "EasyEffects")
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None
    // flags: Qt.WA_DeleteOnClose
    onWidthChanged: {
        EEdb.width = applicationWindow().width;
    }
    onHeightChanged: {
        EEdb.height = applicationWindow().height;
    }
    onVisibleChanged: {
        if (appWindow.visible) {
            switch (EEdb.visiblePage) {
            case 0:
                pageStack.push("qrc:ui/PageStreamsEffects.qml", {
                    "pageType": 0,
                    "streamDB": EEdbStreamOutputs,
                    "visible": true
                });
                break;
            case 1:
                pageStack.push("qrc:ui/PageStreamsEffects.qml", {
                    "pageType": 1,
                    "streamDB": EEdbStreamInputs,
                    "visible": true
                });
                break;
            case 2:
                pageStack.push("qrc:ui/PipeWirePage.qml");
                break;
            default:
                null;
            }
        } else {
            EEdb.save();
            EEdbSpectrum.save();
            EEdbStreamOutputs.save();
            EEdbStreamInputs.save();
            pageStack.pop();
        }
    }
    onClosing: {
        console.log("main window is closing");
    }
    Component.onDestruction: {
        console.log("main window destroyed");
    }

    PreferencesSheet {
        id: preferencesSheet
    }

    Kirigami.Dialog {
        id: aboutDialog

        Kirigami.AboutPage {
            implicitWidth: Kirigami.Units.gridUnit * 30
            implicitHeight: Kirigami.Units.gridUnit * 30
            aboutData: AboutEE
        }

    }

    SystemTrayIcon {
        id: tray

        visible: EEdb.showTrayIcon && canUseSysTray
        icon.name: "com.github.wwmm.easyeffects"
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
            visible: false

            MenuItem {
                text: i18n("Preset: " + EEdb.lastUsedPreset)
                enabled: false
            }

            MenuSeparator {
            }

            MenuItem {
                text: i18n("Quit")
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
                actions: [
                    Kirigami.Action {
                        text: i18n("Presets")
                        icon.name: "bookmarks-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        onTriggered: {
                            showPassiveNotification("Preset Menu!");
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Turn Effects On/Off")
                        icon.name: "system-shutdown-symbolic"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        checkable: true
                        checked: !EEdb.bypass
                        onTriggered: {
                            showPassiveNotification("Turn Effects On/Off");
                            if (checked !== !EEdb.bypass)
                                EEdb.bypass = !checked;

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
                        text: i18n("Output")
                        checkable: true
                        checked: EEdb.visiblePage === 0
                        onTriggered: {
                            pageStack.replace("qrc:ui/PageStreamsEffects.qml", {
                                "pageType": 0,
                                "streamDB": EEdbStreamOutputs,
                                "visible": true
                            });
                            EEdb.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "audio-input-microphone-symbolic"
                        text: i18n("Input")
                        checkable: true
                        checked: EEdb.visiblePage === 1
                        onTriggered: {
                            pageStack.replace("qrc:ui/PageStreamsEffects.qml", {
                                "pageType": 1,
                                "streamDB": EEdbStreamInputs,
                                "visible": true
                            });
                            EEdb.visiblePage = 1;
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "network-server-symbolic"
                        text: i18n("PipeWire")
                        checkable: true
                        checked: EEdb.visiblePage === 2
                        onTriggered: {
                            pageStack.replace("qrc:ui/PipeWirePage.qml");
                            EEdb.visiblePage = 2;
                        }
                    }
                ]
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                actions: [
                    Kirigami.Action {
                        text: i18n("Preferences")
                        icon.name: "gtk-preferences-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            preferencesSheet.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("About EasyEffects")
                        icon.name: "com.github.wwmm.easyeffects"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            aboutDialog.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Quit")
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
