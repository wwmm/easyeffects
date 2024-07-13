import QtCharts
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    // Controls.StackView {
    //     id: stack
    //     anchors.fill: parent
    //     implicitHeight: stack.currentItem.implicitHeight
    //     implicitWidth: Kirigami.Units.gridUnit * 30
    //     initialItem: ColumnLayout {
    //         anchors.fill: parent
    //     }
    // }

    property int pageType: 0 // 0 for output and 1 for input

    Kirigami.CardsListView {
        id: listView

        clip: true
        reuseItems: true

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count === 0
            text: i18n("No Preset")
        }

        delegate: StreamsListDelegate {
        }

        model: ListModel {
            ListElement {
                iconName: "multimedia-player-symbolic"
                appName: "Stream 1"
                mediaName: "test stream"
                streamState: "Idle"
                streamFormat: "F32"
                streamRate: "44.1 kHz"
                streamChannels: "6 channels"
                streamLatency: "30 ms"
                streamMuted: false
                streamVolume: 85
            }

            ListElement {
                iconName: "multimedia-player-symbolic"
                appName: "Stream 2"
                mediaName: "test stream"
                streamState: "Idle"
                streamFormat: "F32"
                streamRate: "48 kHz"
                streamChannels: "2 channels"
                streamLatency: "50 ms"
                streamMuted: true
                streamVolume: 35
            }

        }

    }

    header: EeChart {
        seriesType: EEdbSpectrum.spectrumShape
        useOpenGL: EEdbSpectrum.useOpenGL
    }

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
                    },
                    Kirigami.Action {
                        text: "0,0 ms"
                        enabled: false
                    },
                    Kirigami.Action {
                        text: "0 0 dB"
                        enabled: false
                    }
                ]
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignHCenter
                actions: [
                    Kirigami.Action {
                        // checked: outputPage.visible

                        icon.name: pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                        text: "Players"
                        checkable: true
                    },
                    Kirigami.Action {
                        // checked: inputPage.visible

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
