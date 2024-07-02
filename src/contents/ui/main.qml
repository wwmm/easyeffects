import AboutEE
import EEdb
import Qt.labs.platform
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ApplicationWindow {
    id: root

    width: EEdb.width
    height: EEdb.height
    pageStack.initialPage: outputTab
    pageStack.globalToolBar.style: Kirigami.Settings.isMobile ? Kirigami.ApplicationHeaderStyle.Titles : Kirigami.ApplicationHeaderStyle.Auto
    title: i18nc("@title:window", "EasyEffects")
    onWidthChanged: {
        EEdb.width = applicationWindow().width;
    }
    onHeightChanged: {
        EEdb.height = applicationWindow().height;
    }
    onVisibleChanged: {
        if (!root.visible)
            EEdb.save();

    }

    OutputTab {
        id: outputTab

        visible: true
    }

    InputTab {
        id: intputTab

        visible: false
    }

    PipeWireTab {
        id: pipewireTab

        visible: false
    }

    Kirigami.Dialog {
        id: aboutDialog

        Kirigami.AboutPage {
            implicitWidth: Kirigami.Units.gridUnit * 24
            implicitHeight: Kirigami.Units.gridUnit * 21
            aboutData: AboutEE
        }

    }

    Kirigami.OverlayDrawer {
        id: progressBottomDrawer

        edge: Qt.BottomEdge
        modal: false
        parent: applicationWindow().overlay
        drawerOpen: false

        contentItem: RowLayout {
            Controls.ProgressBar {
                from: 0
                to: 100
                indeterminate: true
                Layout.fillWidth: true
            }

        }

    }

    SystemTrayIcon {
        id: tray

        visible: EEdb.showTrayIcon
        icon.name: "easyeffects"
        onActivated: {
            if (!root.visible) {
                root.show();
                root.raise();
                root.requestActivate();
            } else {
                root.hide();
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

    globalDrawer: Kirigami.GlobalDrawer {
        id: globalDrawer

        drawerOpen: true
        showHeaderWhenCollapsed: true
        collapsible: true
        modal: Kirigami.Settings.isMobile ? true : false
        actions: [
            Kirigami.Action {
                text: outputTab.title
                icon.name: "document-properties-symbolic"
                checked: outputTab.visible
                onTriggered: {
                    if (!outputTab.visible) {
                        while (pageStack.depth > 0)pageStack.pop()
                        pageStack.push(outputTab);
                    }
                }
            },
            Kirigami.Action {
                text: intputTab.title
                icon.name: "dialog-scripts"
                checked: intputTab.visible
                onTriggered: {
                    if (!intputTab.visible) {
                        while (pageStack.depth > 0)pageStack.pop()
                        pageStack.push(intputTab);
                    }
                }
            },
            Kirigami.Action {
                text: pipewireTab.title
                icon.name: "show-gpu-effects-symbolic"
                checked: pipewireTab.visible
                onTriggered: {
                    if (!pipewireTab.visible) {
                        while (pageStack.depth > 0)pageStack.pop()
                        pageStack.push(pipewireTab);
                    }
                }
            }
        ]

        header: Kirigami.AbstractApplicationHeader {

            contentItem: Kirigami.ActionToolBar {
                actions: [
                    Kirigami.Action {
                        text: i18n("Apply")
                        icon.name: "dialog-ok-apply-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        onTriggered: {
                            progressBottomDrawer.open();
                            FGPresetsBackend.applySettings();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Presets")
                        icon.name: "bookmarks-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        onTriggered: {
                            presetsMenu.open();
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Preferences")
                        icon.name: "gtk-preferences"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            root.pageStack.layers.push(preferencesPage);
                        }
                    },
                    Kirigami.Action {
                        text: i18n("About EasyEffects")
                        icon.name: "easyeffects"
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

                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

            }

        }

        footer: Controls.ToolBar {

            contentItem: RowLayout {
                Kirigami.ActionTextField {
                    id: executableName

                    visible: !globalDrawer.collapsed
                    Layout.fillWidth: true
                    placeholderText: i18n("Executable Name")
                    rightActions: [
                        Kirigami.Action {
                            icon.name: "edit-clear"
                            onTriggered: {
                                executableName.text = "";
                                executableName.accepted();
                            }
                        }
                    ]
                }

            }

        }

    }

}
