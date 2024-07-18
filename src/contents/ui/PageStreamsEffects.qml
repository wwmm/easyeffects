import QtCharts
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    //To do: the bypass needs to be set from the corresponding plugin database

    property int pageType: 0 // 0 for output and 1 for input
    property var streamDB

    padding: 0
    Component.onCompleted: {
        let plugins = streamDB.plugins;
        for (let n = 0; n < plugins.length; n++) {
            pluginsListModel.append({
                "name": plugins[n],
                "translatedName": "",
                "bypass": false
            });
        }
    }

    Connections {
        function onPluginsChanged() {
            console.log("new plugins list: " + streamDB.plugins);
        }

        target: streamDB
    }

    Connections {
        function onDataChanged() {
            let newList = [];
            for (let n = 0; n < pluginsListModel.count; n++) {
                newList.push(pluginsListModel.get(n).name);
            }
            streamDB.plugins = newList;
        }

        target: pluginsListModel
    }

    MenuAddPlugins {
        id: menuAddPlugins
    }

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

                delegate: DelegateStreamsList {
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
            columns: 3
            rows: 1
            columnSpacing: 0

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
                                menuAddPlugins.open();
                            }
                        }
                    ]
                }

                ListView {
                    id: pluginsListView

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    reuseItems: true

                    delegate: DelegatePluginsList {
                    }

                    moveDisplaced: Transition {
                        YAnimator {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }

                    }

                    model: ListModel {
                        id: pluginsListModel
                    }

                    header: RowLayout {
                        visible: pluginsListView.count !== 0

                        Kirigami.Icon {
                            source: pageType === 0 ? "source-playlist-symbolic" : "audio-input-microphone-symbolic"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                            Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                            Layout.leftMargin: Kirigami.Units.mediumSpacing
                            enabled: false
                        }

                        Controls.Label {
                            text: pageType === 0 ? i18n("Players") : i18n("Input Device")
                            enabled: false
                        }

                    }

                    footer: RowLayout {
                        visible: pluginsListView.count !== 0

                        Kirigami.Icon {
                            source: pageType === 0 ? "audio-speakers-symbolic" : "source-playlist-symbolic"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                            Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                            Layout.leftMargin: Kirigami.Units.mediumSpacing
                            enabled: false
                        }

                        Controls.Label {
                            text: pageType === 0 ? "Output Device" : i18n("Recorders")
                            enabled: false
                        }

                    }

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
