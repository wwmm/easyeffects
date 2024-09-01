import "Common.js" as Common
import EEpw
import EEtagsPluginName
import QtCharts
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    //To do: the bypass needs to be set from the corresponding plugin database

    id: pageStreamsEffects

    property int pageType: 0 // 0 for output and 1 for input
    property var streamDB
    property string logTag: "PageStreamsEffects"

    padding: 0
    Component.onCompleted: {
        switch (streamDB.visiblePage) {
        case 0:
            stackPages.push(pageStreams);
            break;
        case 1:
            stackPages.push(pagePlugins);
            break;
        default:
        }
    }

    MenuAddPlugins {
        id: menuAddPlugins

        streamDB: pageStreamsEffects.streamDB
    }

    Component {
        id: pageStreams

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: streamsListView

                clip: true
                reuseItems: true
                model: pageType === 0 ? ModelOutputStreams : ModelInputStreams

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: streamsListView.count === 0
                    text: i18n("Empty List")
                    explanation: i18n("No Audio Application Available")
                    icon.name: "emblem-music-symbolic"
                }

                delegate: DelegateStreamsList {
                }

            }

        }

    }

    Component {
        id: pagePlugins

        GridLayout {
            function populatePluginsListModel(plugins) {
                let names = PluginsNameModel.getBaseNames();
                for (let n = 0; n < plugins.length; n++) {
                    for (let k = 0; k < names.length; k++) {
                        if (plugins[n].startsWith(names[k])) {
                            pluginsListModel.append({
                                "name": plugins[n],
                                "baseName": names[k],
                                "translatedName": PluginsNameModel.translate(names[k]),
                                "bypass": false
                            });
                            createPluginStack(names[k]);
                            break;
                        }
                    }
                }
            }

            function createPluginStack(baseName) {
                switch (baseName) {
                case BasePluginName.autogain:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Autogain.qml");
                    break;
                default:
                    console.log(logTag + " invalid plugin name: " + baseName);
                }
            }

            Component.onCompleted: {
                populatePluginsListModel(streamDB.plugins);
            }
            Layout.fillHeight: true
            Layout.fillWidth: true
            columns: 3
            rows: 1
            columnSpacing: 0

            Connections {
                function onPluginsChanged() {
                    const newList = streamDB.plugins;
                    let currentList = [];
                    for (let n = 0; n < pluginsListModel.count; n++) {
                        currentList.push(pluginsListModel.get(n).name);
                    }
                    if (Common.equalArrays(newList, currentList))
                        return ;

                    pluginsListModel.clear();
                    populatePluginsListModel(newList);
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

            ColumnLayout {
                // Layout.horizontalStretchFactor: 1
                // Layout.fillWidth: true
                implicitWidth: pluginsListView.width

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
                    // Layout.fillWidth: true
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    reuseItems: true

                    model: ListModel {
                        id: pluginsListModel
                    }

                    delegate: DelegatePluginsList {
                    }

                    moveDisplaced: Transition {
                        YAnimator {
                            duration: Kirigami.Units.longDuration
                            easing.type: Easing.InOutQuad
                        }

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
                Layout.horizontalStretchFactor: 3
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

    Controls.StackView {
        id: stackPages

        anchors.fill: parent
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
                        checked: streamDB.visiblePage === 0
                        onTriggered: {
                            stackPages.replace(pageStreams);
                            streamDB.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        icon.name: "emblem-music-symbolic"
                        text: "Effects"
                        checkable: true
                        checked: streamDB.visiblePage === 1
                        onTriggered: {
                            stackPages.replace(pagePlugins);
                            streamDB.visiblePage = 1;
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
