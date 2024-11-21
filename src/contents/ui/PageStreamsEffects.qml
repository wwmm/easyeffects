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
    required property var pipelineInstance
    property string logTag: "PageStreamsEffects"
    property int minLeftLevel: -99
    property int minRightLevel: -99

    padding: 0
    Component.onCompleted: {
        switch (streamDB.visiblePage) {
        case 0:
            stackPages.replace(pageStreams);
            break;
        case 1:
            stackPages.replace(pagePlugins);
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

            function createPluginStack(name, baseName, pluginDB) {
                switch (baseName) {
                case BasePluginName.autogain:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Autogain.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.bassEnhancer:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/BassEnhancer.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.compressor:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Compressor.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.crystalizer:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Crystalizer.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.exciter:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Exciter.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.filter:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Filter.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.gate:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Gate.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.limiter:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Limiter.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.maximizer:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/Maximizer.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                case BasePluginName.stereoTools:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    pluginsStack.push("qrc:ui/StereoTools.qml", {
                        "name": name,
                        "pluginDB": pluginDB,
                        "pipelineInstance": pipelineInstance
                    });
                    break;
                default:
                    while (pluginsStack.depth > 1)pluginsStack.pop()
                    console.log(logTag + " invalid plugin name: " + baseName);
                }
            }

            Component.onCompleted: {
                pluginsListView.currentIndex = -1;
                populatePluginsListModel(streamDB.plugins);
                if (streamDB.plugins.length > 0) {
                    if (!streamDB.plugins.includes(streamDB.visiblePlugin))
                        streamDB.visiblePlugin = streamDB.plugins[0];

                    pluginsListView.currentIndex = streamDB.plugins.findIndex((v) => {
                        return v == streamDB.visiblePlugin;
                    });
                    let baseNames = PluginsNameModel.getBaseNames();
                    for (let k = 0; k < baseNames.length; k++) {
                        if (streamDB.visiblePlugin.startsWith(baseNames[k])) {
                            createPluginStack(streamDB.visiblePlugin, baseNames[k], pluginsDB[streamDB.visiblePlugin]);
                            break;
                        }
                    }
                }
                frameAnimation.start();
            }
            Component.onDestruction: {
                frameAnimation.stop();
            }
            Layout.fillHeight: true
            Layout.fillWidth: true
            columns: 3
            rows: 1
            columnSpacing: 0

            FrameAnimation {
                id: frameAnimation

                onTriggered: {
                    if (pluginsStack.depth > 1)
                        pluginsStack.currentItem.updateMeters();

                }
            }

            Connections {
                function onPipelineChanged() {
                    const newList = streamDB.plugins;
                    let currentList = [];
                    for (let n = 0; n < pluginsListModel.count; n++) {
                        currentList.push(pluginsListModel.get(n).name);
                    }
                    if (Common.equalArrays(newList, currentList))
                        return ;

                    pluginsListModel.clear();
                    populatePluginsListModel(newList);
                    if (newList.length === 1 && pluginsListView.currentIndex === -1)
                        pluginsListView.currentIndex = 0;

                }

                target: pipelineInstance
            }

            Connections {
                function onDataChanged() {
                    let newList = [];
                    for (let n = 0; n < pluginsListModel.count; n++) {
                        newList.push(pluginsListModel.get(n).name);
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

                Controls.Button {
                    Layout.margins: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignHCenter
                    text: i18n("Add Effect")
                    icon.name: "list-add"
                    onClicked: menuAddPlugins.open()
                }

                ListView {
                    id: pluginsListView

                    Layout.fillHeight: true
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    reuseItems: true
                    onCurrentItemChanged: {
                        if (pluginsListView.currentItem) {
                            let name = pluginsListView.currentItem.name;
                            if (streamDB.visiblePlugin !== name) {
                                streamDB.visiblePlugin = name;
                                let baseName = pluginsListModel.get(pluginsListView.currentIndex).baseName;
                                createPluginStack(name, baseName, pluginsDB[name]);
                            }
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
                Component.onCompleted: {
                    footerFrameAnimation.start();
                }
                Component.onDestruction: {
                    footerFrameAnimation.stop();
                }
                actions: [
                    Kirigami.Action {
                        id: actionRateValue

                        text: "kHz"
                        enabled: false
                    },
                    Kirigami.Action {
                        id: actionLatencyValue

                        text: "0,0 ms"
                        enabled: false
                    },
                    Kirigami.Action {
                        id: actionLevelValue

                        text: "0 0 dB"
                        enabled: false
                    }
                ]

                FrameAnimation {
                    id: footerFrameAnimation

                    onTriggered: {
                        let left = Number(pipelineInstance.getOutputLevelLeft());
                        let right = Number(pipelineInstance.getOutputLevelRight());
                        if (isNaN(left) || left < minLeftLevel)
                            left = minLeftLevel;

                        if (isNaN(right) || right < minRightLevel)
                            right = minRightLevel;

                        const localeLeft = left.toLocaleString(Qt.locale(), 'f', 0);
                        const localeRight = right.toLocaleString(Qt.locale(), 'f', 0);
                        const latency = Number(pipelineInstance.getPipeLineLatency()).toLocaleString(Qt.locale(), 'f', 1);
                        const rate = Number(pipelineInstance.getPipeLineRate()).toLocaleString(Qt.locale(), 'f', 1);
                        actionLevelValue.text = `${localeLeft} ${localeRight} dB`;
                        actionLatencyValue.text = `${latency} ms`;
                        actionRateValue.text = `${rate} kHz`;
                    }
                }

            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignHCenter
                position: Controls.ToolBar.Footer
                actions: [
                    Kirigami.Action {
                        icon.name: pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                        text: pageType === 0 ? i18n("Players") : i18n("Recorders")
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
