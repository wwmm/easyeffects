import QtCharts
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    property int pageType: 0 // 0 for output and 1 for input

    padding: 0

    StackLayout {
        id: stackLayout

        anchors.fill: parent
        currentIndex: 0

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: streamsListView

                clip: true
                reuseItems: true

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: streamsListView.count === 0
                    text: i18n("No Preset")
                }

                delegate: StreamsListDelegate {
                }

                model: ListModel {
                    ListElement {
                        iconName: "firefox"
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
                        iconName: "chromium"
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

        }

        GridLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            columns: 3
            rows: 1

            ColumnLayout {
                Kirigami.ActionToolBar {
                    Layout.margins: Kirigami.Units.smallSpacing
                    alignment: Qt.AlignHCenter
                    flat: false
                    actions: [
                        Kirigami.Action {
                            text: i18n("Add Effect")
                            icon.name: "list-add"
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                                showPassiveNotification("Adding a Plugin");
                            }
                        }
                    ]
                }

                ListView {
                    id: pluginsListView

                    Layout.fillHeight: true
                }

            }

            Kirigami.Separator {
                Layout.fillHeight: true
                visible: true
            }

            Controls.StackView {
                id: pluginsStack

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.minimumWidth: pluginsStack.currentItem.implicitWidth

                initialItem: Kirigami.Page {
                    anchors.fill: parent

                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        text: i18n("No Effects")
                        explanation: i18n("Audio Stream Not Modified")
                        icon.name: "emblem-music-symbolic"
                    }

                }

            }

        }

    }

    header: EeChart {
        id: spectrumChart

        implicitHeight: EEdbSpectrum.height
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
                        icon.name: pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                        text: "Players"
                        checkable: true
                        checked: stackLayout.currentIndex === 0
                        onTriggered: {
                            if (checked === true)
                                stackLayout.currentIndex = 0;

                        }
                    },
                    Kirigami.Action {
                        icon.name: "emblem-music-symbolic"
                        text: "Effects"
                        checkable: true
                        checked: stackLayout.currentIndex === 1
                        onTriggered: {
                            if (checked === true)
                                stackLayout.currentIndex = 1;

                        }
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
