import "Common.js" as Common
import EEdbm
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

    required property int pageType // 0 for output and 1 for input
    required property var streamDB
    required property var pluginsDB
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
                let baseNames = PluginsNameModel.getBaseNames();
                for (let n = 0; n < plugins.length; n++) {
                    for (let k = 0; k < baseNames.length; k++) {
                        if (plugins[n].startsWith(baseNames[k])) {
                            pluginsListModel.append({
                                "name": plugins[n],
                                "baseName": baseNames[k],
                                "translatedName": PluginsNameModel.translate(baseNames[k]),
                                "bypass": false
                            });
                            break;
                        }
                    }
                }
            }

            function createPluginStack(baseName, pluginDB) {
                switch (baseName) {
                case BasePluginName.autogain:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Autogain.qml", {
                        "pluginDB": pluginDB
                    });
                    break;
                default:
                    console.log(logTag + " invalid plugin name: " + baseName);
                }
            }

            Component.onCompleted: {
                populatePluginsListModel(streamDB.plugins);
                if (streamDB.plugins.length > 0) {
                    let firstPlugin = streamDB.plugins[0];
                    let baseNames = PluginsNameModel.getBaseNames();
                    for (let k = 0; k < baseNames.length; k++) {
                        if (firstPlugin.startsWith(baseNames[k])) {
                            createPluginStack(baseNames[k], pluginsDB[firstPlugin]);
                            break;
                        }
                    }
                    streamDB.visiblePlugin = firstPlugin;
                }
            }
            Layout.fillHeight: true
            Layout.fillWidth: true
            columns: 3
            rows: 1
            columnSpacing: 0

            Connections {
                function onPluginsChanged() {
                    console.log("hello from qml!!!!!!");
                    const newList = streamDB.plugins;
                    let currentList = [];
                    for (let n = 0; n < pluginsListModel.count; n++) {
                        currentList.push(pluginsListModel.get(n).name);
                    }
                    if (Common.equalArrays(newList, currentList))
                        return ;

                    let currentSelection = pluginsListModel.get(pluginsListView.currentIndex);
                    if (pluginsListModel.count > 0)
                        currentSelection = pluginsListModel.get(pluginsListView.currentIndex);

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
                    if (pluginsListModel.currentItem !== null) {
                        let name = pluginsListView.currentItem.name;
                        if (streamDB.visiblePlugin !== name) {
                            streamDB.visiblePlugin = name;
                            let baseName = pluginsListModel.get(pluginsListView.currentIndex).baseName;
                            createPluginStack(baseName, pluginsDB[name]);
                        }
                    }
                    if (!Common.equalArrays(streamDB.plugins, newList))
                        streamDB.plugins = newList;

                    if (newList.length === 0) {
                        streamDB.visiblePlugin = "";
                        while (pluginsStack.depth > 1)pluginsStack.pop()
                    }
                }

                target: pluginsListModel
            }

            ColumnLayout {
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
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    reuseItems: true
                    onCurrentIndexChanged: {
                        if (pluginsListModel.currentItem !== null) {
                            let name = pluginsListView.currentItem.name;
                            if (streamDB.visiblePlugin !== name) {
                                streamDB.visiblePlugin = name;
                                let baseName = pluginsListModel.get(pluginsListView.currentIndex).baseName;
                                createPluginStack(baseName, pluginsDB[name]);
                            }
                            showPassiveNotification("Clicked on plugin: " + name);
                        }
                    }

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

                initialItem: Kirigami.ScrollablePage {
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

        implicitHeight: EEdbm.spectrum.height
        seriesType: EEdbm.spectrum.spectrumShape
        useOpenGL: EEdbm.spectrum.useOpenGL
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
                position: Controls.ToolBar.Footer
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
                position: Controls.ToolBar.Footer
                actions: [
                    Kirigami.Action {
                        icon.name: pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                        text: i18n("Players")
                        checkable: true
                        checked: streamDB.visiblePage === 0
                        onTriggered: {
                            stackPages.replace(pageStreams);
                            streamDB.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        icon.name: "emblem-music-symbolic"
                        text: i18n("Effects")
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
                position: Controls.ToolBar.Footer
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
