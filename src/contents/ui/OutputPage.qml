import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {

    footer: Kirigami.AbstractApplicationHeader {

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
                        text: "kHz"
                        enabled: false
                        displayHint: Kirigami.DisplayHint.KeepVisible
                    },
                    Kirigami.Action {
                        text: "0,0 ms"
                        enabled: false
                        displayHint: Kirigami.DisplayHint.KeepVisible
                    },
                    Kirigami.Action {
                        text: "0 0 dB"
                        enabled: false
                        displayHint: Kirigami.DisplayHint.KeepVisible
                    }
                ]
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignHCenter
                actions: [
                    Kirigami.Action {
                        // checked: outputPage.visible

                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "multimedia-player-symbolic"
                        text: "Players"
                        checkable: true
                    },
                    Kirigami.Action {
                        // checked: inputPage.visible

                        displayHint: Kirigami.DisplayHint.KeepVisible
                        icon.name: "emblem-music-symbolic"
                        text: "Effects"
                        checkable: true
                    }
                ]
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                actions: [
                    Kirigami.Action {
                        text: i18n("Excluded Apps")
                        icon.name: "im-ban-kick-user-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        onTriggered: {
                            showPassiveNotification("Blocklist Menu!");
                        }
                    }
                ]
            }

        }

    }

}
