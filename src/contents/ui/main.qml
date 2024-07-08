import AboutEE
import EEdb
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
    pageStack.initialPage: outputPage
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None
    onWidthChanged: {
        EEdb.width = applicationWindow().width;
    }
    onHeightChanged: {
        EEdb.height = applicationWindow().height;
    }
    onVisibleChanged: {
        if (!appWindow.visible)
            EEdb.save();

    }

    OutputPage {
        id: outputPage

        visible: true
    }

    InputPage {
        id: inputPage

        visible: false
    }

    PipeWirePage {
        id: pipeWirePage

        visible: false
    }

    PreferencesSheet {
        id: preferencesSheet
    }

    Kirigami.Dialog {
        id: aboutDialog

        Kirigami.AboutPage {
            implicitWidth: Kirigami.Units.gridUnit * 24
            implicitHeight: Kirigami.Units.gridUnit * 21
            aboutData: AboutEE
        }

    }

    SystemTrayIcon {
        id: tray

        visible: EEdb.showTrayIcon
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
                        onTriggered: {
                            showPassiveNotification("Turn Effects On/Off");
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
                        text: "Output"
                        checkable: true
                        checked: outputPage.visible
                        onTriggered: {
                            if (!outputPage.visible) {
                                while (pageStack.depth > 0)pageStack.pop()
                                pageStack.push(outputPage);
                            }
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "audio-input-microphone-symbolic"
                        text: "Input"
                        checkable: true
                        checked: inputPage.visible
                        onTriggered: {
                            if (!inputPage.visible) {
                                while (pageStack.depth > 0)pageStack.pop()
                                pageStack.push(inputPage);
                            }
                        }
                    },
                    Kirigami.Action {
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "network-server-symbolic"
                        text: "PipeWire"
                        checkable: true
                        checked: pipeWirePage.visible
                        onTriggered: {
                            if (!pipeWirePage.visible) {
                                while (pageStack.depth > 0)pageStack.pop()
                                pageStack.push(pipeWirePage);
                            }
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
