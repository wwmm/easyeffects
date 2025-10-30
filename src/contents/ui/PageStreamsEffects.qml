pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.Page {
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
        updateStackPages();
    }

    function updateStackPages() {
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

    Connections {
        function onVisiblePageChanged() {
            pageStreamsEffects.updateStackPages();
        }

        target: pageStreamsEffects.streamDB
    }

    ListModel {
        id: pluginsListModel
    }

    MenuAddPlugins {
        id: menuAddPlugins

        streamDB: pageStreamsEffects.streamDB
    }

    BlocklistSheet {
        id: blocklistSheet

        streamDB: pageStreamsEffects.streamDB
    }

    Component {
        id: pageStreams

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: streamsListView

                clip: true
                reuseItems: true
                model: pageStreamsEffects.pageType === 0 ? PW.ModelOutputStreams : PW.ModelInputStreams

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: streamsListView.count === 0
                    text: i18n("Empty List")// qmllint disable
                    explanation: i18n("No Audio Application Available")// qmllint disable
                    icon.name: "folder-music-symbolic"
                }

                delegate: DelegateStreamsList {}
            }
        }
    }

    Component {
        id: pagePlugins

        GridLayout {
            id: pagePluginsGrid

            function populatePluginsListModel(plugins) {
                let baseNames = TagsPluginName.PluginsNameModel.getBaseNames();
                pluginsListModel.clear();

                for (let n = 0; n < plugins.length; n++) {
                    for (let k = 0; k < baseNames.length; k++) {
                        if (plugins[n].startsWith(baseNames[k])) {
                            pluginsListModel.append({
                                "name": plugins[n],
                                "baseName": baseNames[k],
                                "translatedName": TagsPluginName.PluginsNameModel.translate(baseNames[k]),
                                "pluginDB": pageStreamsEffects.pluginsDB[plugins[n]]
                            });
                            break;
                        }
                    }
                }
            }

            function createPluginStack(name, baseName, pluginDB) {
                while (pluginsStack.depth > 1)
                    pluginsStack.pop();

                const pluginMap = {
                    [TagsPluginName.BaseName.autogain]: "qrc:/ui/Autogain.qml",
                    [TagsPluginName.BaseName.bassEnhancer]: "qrc:/ui/BassEnhancer.qml",
                    [TagsPluginName.BaseName.bassLoudness]: "qrc:/ui/BassLoudness.qml",
                    [TagsPluginName.BaseName.compressor]: "qrc:/ui/Compressor.qml",
                    [TagsPluginName.BaseName.convolver]: "qrc:/ui/Convolver.qml",
                    [TagsPluginName.BaseName.crossfeed]: "qrc:/ui/Crossfeed.qml",
                    [TagsPluginName.BaseName.crystalizer]: "qrc:/ui/Crystalizer.qml",
                    [TagsPluginName.BaseName.delay]: "qrc:/ui/Delay.qml",
                    [TagsPluginName.BaseName.deepfilternet]: "qrc:/ui/DeepFilterNet.qml",
                    [TagsPluginName.BaseName.deesser]: "qrc:/ui/Deesser.qml",
                    [TagsPluginName.BaseName.equalizer]: "qrc:/ui/Equalizer.qml",
                    [TagsPluginName.BaseName.exciter]: "qrc:/ui/Exciter.qml",
                    [TagsPluginName.BaseName.echoCanceller]: "qrc:/ui/EchoCanceller.qml",
                    [TagsPluginName.BaseName.expander]: "qrc:/ui/Expander.qml",
                    [TagsPluginName.BaseName.filter]: "qrc:/ui/Filter.qml",
                    [TagsPluginName.BaseName.gate]: "qrc:/ui/Gate.qml",
                    [TagsPluginName.BaseName.levelMeter]: "qrc:/ui/LevelMeter.qml",
                    [TagsPluginName.BaseName.limiter]: "qrc:/ui/Limiter.qml",
                    [TagsPluginName.BaseName.loudness]: "qrc:/ui/Loudness.qml",
                    [TagsPluginName.BaseName.maximizer]: "qrc:/ui/Maximizer.qml",
                    [TagsPluginName.BaseName.multibandCompressor]: "qrc:/ui/MultibandCompressor.qml",
                    [TagsPluginName.BaseName.multibandGate]: "qrc:/ui/MultibandGate.qml",
                    [TagsPluginName.BaseName.pitch]: "qrc:/ui/Pitch.qml",
                    [TagsPluginName.BaseName.reverb]: "qrc:/ui/Reverb.qml",
                    [TagsPluginName.BaseName.rnnoise]: "qrc:/ui/RNNoise.qml",
                    [TagsPluginName.BaseName.speex]: "qrc:/ui/Speex.qml",
                    [TagsPluginName.BaseName.stereoTools]: "qrc:/ui/StereoTools.qml"
                };

                const componentUrl = pluginMap[baseName];

                if (!componentUrl) {
                    console.log(pageStreamsEffects.logTag + " invalid plugin name: " + baseName);

                    return;
                }

                if (baseName !== TagsPluginName.BaseName.equalizer) {
                    pluginsStack.push(componentUrl, {
                        name: name,
                        pluginDB: pluginDB,
                        pipelineInstance: pageStreamsEffects.pipelineInstance
                    });
                } else {
                    pluginsStack.push(componentUrl, {
                        name: name,
                        pluginDB: pluginDB,
                        pipelineInstance: pageStreamsEffects.pipelineInstance,
                        leftDB: pageStreamsEffects.pluginsDB[name + "#left"],
                        rightDB: pageStreamsEffects.pluginsDB[name + "#right"]
                    });
                }
            }

            function populatePluginsStack() {
                if (pageStreamsEffects.streamDB.plugins.length > 0) {
                    if (!pageStreamsEffects.streamDB.plugins.includes(pageStreamsEffects.streamDB.visiblePlugin))
                        pageStreamsEffects.streamDB.visiblePlugin = pageStreamsEffects.streamDB.plugins[0];

                    pluginsListView.currentIndex = pageStreamsEffects.streamDB.plugins.findIndex(v => {
                        return v === pageStreamsEffects.streamDB.visiblePlugin;
                    });

                    const baseNames = TagsPluginName.PluginsNameModel.getBaseNames();

                    for (let k = 0; k < baseNames.length; k++) {
                        if (pageStreamsEffects.streamDB.visiblePlugin.startsWith(baseNames[k])) {
                            createPluginStack(pageStreamsEffects.streamDB.visiblePlugin, baseNames[k], pageStreamsEffects.pluginsDB[pageStreamsEffects.streamDB.visiblePlugin]);
                            break;
                        }
                    }
                }
            }

            Component.onCompleted: {
                pluginsListView.currentIndex = -1;

                populatePluginsListModel(pageStreamsEffects.streamDB.plugins);

                populatePluginsStack();
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

                running: pageStreamsEffects.pageType !== 2 && appWindow.visible // qmllint disable

                onTriggered: {
                    if (pluginsStack.depth > 1)
                        pluginsStack.currentItem.updateMeters();
                }
            }

            Connections {
                function onPipelineChanged() {
                    const newList = pageStreamsEffects.streamDB.plugins;

                    pagePluginsGrid.populatePluginsListModel(newList);

                    if (newList.length === 0) {
                        pageStreamsEffects.streamDB.visiblePlugin = "";

                        while (pluginsStack.depth > 1)
                            pluginsStack.pop();
                    }

                    if (newList.length === 1 && pluginsListView.currentIndex === -1)
                        pluginsListView.currentIndex = 0;

                    pagePluginsGrid.populatePluginsStack();
                }

                target: pageStreamsEffects.pipelineInstance
            }

            Connections {
                function onDataChanged() {
                    let newList = [];

                    for (let n = 0; n < pluginsListModel.count; n++) {
                        newList.push(pluginsListModel.get(n).name);
                    }

                    if (!Common.equalArrays(pageStreamsEffects.streamDB.plugins, newList))
                        pageStreamsEffects.streamDB.plugins = newList;

                    if (newList.length === 0) {
                        pageStreamsEffects.streamDB.visiblePlugin = "";

                        while (pluginsStack.depth > 1)
                            pluginsStack.pop();
                    }
                }

                target: pluginsListModel
            }

            ColumnLayout {
                implicitWidth: pluginsListView.width

                Controls.Button {
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignHCenter
                    text: DB.Manager.main.collapsePluginsList === true ? "" : i18n("Add Effect") // qmllint disable
                    icon.name: "list-add"
                    onClicked: menuAddPlugins.open()
                }

                ListView {
                    id: pluginsListView

                    Layout.fillHeight: true
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    reuseItems: true
                    model: pluginsListModel

                    delegate: DelegatePluginsList {
                        listModel: pluginsListModel
                        listView: pluginsListView
                        streamDB: pageStreamsEffects.streamDB
                        onSelectedChanged: name => {
                            if (pageStreamsEffects.streamDB.visiblePlugin !== name) {
                                pageStreamsEffects.streamDB.visiblePlugin = name;

                                let baseName = pluginsListModel.get(pluginsListView.currentIndex).baseName;

                                pagePluginsGrid.createPluginStack(name, baseName, pageStreamsEffects.pluginsDB[name]);
                            }
                        }
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
                            source: pageStreamsEffects.pageType === 0 ? "source-playlist-symbolic" : "audio-input-microphone-symbolic"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            Layout.leftMargin: Kirigami.Units.mediumSpacing
                        }

                        Controls.Label {
                            text: pageStreamsEffects.pageType === 0 ? i18n("Players") : i18n("Input Device")// qmllint disable
                            enabled: false
                            visible: !DB.Manager.main.collapsePluginsList
                        }
                    }

                    footer: RowLayout {
                        visible: pluginsListView.count !== 0

                        Kirigami.Icon {
                            source: pageStreamsEffects.pageType === 0 ? "audio-speakers-symbolic" : "source-playlist-symbolic"
                            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                            Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                            Layout.leftMargin: Kirigami.Units.mediumSpacing
                        }

                        Controls.Label {
                            text: pageStreamsEffects.pageType === 0 ? "Output Device" : i18n("Recorders")// qmllint disable
                            enabled: false
                            visible: !DB.Manager.main.collapsePluginsList
                        }
                    }
                }

                Controls.Button {
                    Layout.topMargin: Kirigami.Units.largeSpacing
                    Layout.bottomMargin: Kirigami.Units.smallSpacing
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignHCenter
                    text: DB.Manager.main.collapsePluginsList === true ? "" : i18n("Close") // qmllint disable
                    icon.name: DB.Manager.main.collapsePluginsList === true ? "sidebar-collapse-right-symbolic" : "sidebar-collapse-symbolic"
                    Controls.ToolTip.text: DB.Manager.main.collapsePluginsList === true ? i18n("Expand the list of effects pipeline") : i18n("Reduce the list of effects pipeline")
                    Controls.ToolTip.visible: hovered
                    onClicked: {
                        DB.Manager.main.collapsePluginsList = !DB.Manager.main.collapsePluginsList;
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
                visible: pageStreamsEffects.streamDB.visiblePage === 1

                initialItem: Kirigami.ScrollablePage {
                    Kirigami.PlaceholderMessage {
                        anchors.centerIn: parent
                        text: i18n("No Effects")// qmllint disable
                        explanation: i18n("Audio Stream Not Modified")// qmllint disable
                        icon.name: "folder-music-symbolic"
                    }
                }

                Binding {
                    target: pluginsStack.currentItem?.pluginBackend ?? null
                    property: "updateLevelMeters"
                    value: pluginsStack.visible
                    when: pluginsStack.currentItem?.pluginBackend !== undefined && pluginsStack.visible !== undefined
                }
            }
        }
    }

    Controls.StackView {
        id: stackPages

        anchors.fill: parent
    }

    header: ColumnLayout {
        spacing: 0

        EeChart {
            id: spectrumChart

            Layout.fillWidth: true
            implicitHeight: DB.Manager.spectrum.height
            seriesType: DB.Manager.spectrum.spectrumShape
            colorScheme: DB.Manager.spectrum.spectrumColorScheme
            colorTheme: DB.Manager.spectrum.spectrumColorTheme
            xMin: DB.Manager.spectrum.minimumFrequency
            xMax: DB.Manager.spectrum.maximumFrequency
            yMin: -100
            yMax: 0
            logarithimicHorizontalAxis: DB.Manager.spectrum.logarithimicHorizontalAxis
            dynamicYScale: DB.Manager.spectrum.dynamicYScale
            xUnit: "Hz"
            visible: DB.Manager.spectrum.state

            Component.onDestruction: {
                headerFrameAnimation.stop();
            }

            FrameAnimation {
                id: headerFrameAnimation

                running: DB.Manager.spectrum.state && appWindow.visible // qmllint disable

                onRunningChanged: {
                    pageStreamsEffects.pipelineInstance.setSpectrumBypass(!running);
                }

                onTriggered: {
                    pageStreamsEffects.pipelineInstance.requestSpectrumData();
                }
            }

            Connections {
                function onNewSpectrumData(newData) {
                    spectrumChart.updateData(newData);
                }

                target: pageStreamsEffects.pipelineInstance
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
            visible: true
        }
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
                overflowIconName: "info-symbolic"
                actions: [
                    Kirigami.Action {
                        id: actionRateValue

                        // Sample rate

                        displayComponent: Controls.Label {
                            text: actionRateValue.text
                            textFormat: Text.RichText
                        }
                    },
                    Kirigami.Action {
                        id: actionLatencyValue

                        // Latency

                        displayComponent: Controls.Label {
                            text: actionLatencyValue.text
                            textFormat: Text.RichText
                        }
                    },
                    Kirigami.Action {
                        id: actionLevelValue

                        // Global Level Meter

                        displayComponent: Controls.Label {
                            text: actionLevelValue.text
                            textFormat: Text.RichText
                            wrapMode: Text.Wrap
                        }
                    },
                    Kirigami.Action {
                        id: actionLevelSaturation

                        icon.name: "dialog-warning-symbolic"
                        visible: false
                    }
                ]

                Component.onDestruction: {
                    pageStreamsEffects.pipelineInstance.setUpdateLevelMeters(false);

                    footerFrameAnimation.stop();
                }

                FrameAnimation {
                    id: footerFrameAnimation

                    running: pageStreamsEffects.pageType !== 2 && appWindow.visible // qmllint disable

                    onRunningChanged: {
                        pageStreamsEffects.pipelineInstance.setUpdateLevelMeters(running);
                    }

                    onTriggered: {
                        let left = Number(pageStreamsEffects.pipelineInstance.getOutputLevelLeft());
                        let right = Number(pageStreamsEffects.pipelineInstance.getOutputLevelRight());
                        if (Number.isNaN(left) || left < pageStreamsEffects.minLeftLevel)
                            left = pageStreamsEffects.minLeftLevel;

                        if (Number.isNaN(right) || right < pageStreamsEffects.minRightLevel)
                            right = pageStreamsEffects.minRightLevel;

                        if ((left > 0 || right > 0) && actionLevelSaturation.visible !== true)
                            actionLevelSaturation.visible = true;
                        else if (actionLevelSaturation.visible !== false)
                            actionLevelSaturation.visible = false;

                        const localeLeft = left.toLocaleString(Qt.locale(), 'f', 0).padStart(3, ' ');
                        const localeRight = right.toLocaleString(Qt.locale(), 'f', 0).padStart(3, ' ');

                        // Since we found the standard bold too heavy for the
                        // bottom bar, we used the font-weight style.
                        const cssFontWeight = `style="font-weight:600"`;

                        const styledLocaleLeft = left > -10 ? `<span ${cssFontWeight}>${localeLeft}</span>` : localeLeft;
                        const styledLocaleRight = right > -10 ? `<span ${cssFontWeight}>${localeRight}</span>` : localeRight;

                        const pipelineLatency = Number(pageStreamsEffects.pipelineInstance.getPipeLineLatency());
                        const latency = pipelineLatency.toLocaleString(Qt.locale(), 'f', 1);
                        const styledLatency = pipelineLatency > 0 ? `<span ${cssFontWeight}>${latency}</span>` : latency;

                        const rate = Number(pageStreamsEffects.pipelineInstance.getPipeLineRate()).toLocaleString(Qt.locale(), 'f', 1);

                        const cssFontColor = `style="color:${Kirigami.Theme.textColor}"`;

                        actionRateValue.text = `<pre ${cssFontColor}> <span ${cssFontWeight}>${rate}</span> kHz </pre>`;
                        actionLatencyValue.text = `<pre ${cssFontColor}> ${styledLatency} ms </pre>`;
                        actionLevelValue.text = `<pre ${cssFontColor}> ${styledLocaleLeft} ${styledLocaleRight} dB</pre>`;
                    }
                }
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignHCenter
                position: Controls.ToolBar.Footer

                Controls.ActionGroup {
                    id: footerActionGroup
                    exclusive: true
                }

                actions: [
                    Kirigami.Action {
                        icon.name: pageStreamsEffects.pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                        text: pageStreamsEffects.pageType === 0 ? i18n("Players") : i18n("Recorders")// qmllint disable
                        checkable: true
                        checked: pageStreamsEffects.streamDB.visiblePage === 0
                        Controls.ActionGroup.group: footerActionGroup
                        onTriggered: {
                            pageStreamsEffects.streamDB.visiblePage = 0;
                        }
                    },
                    Kirigami.Action {
                        icon.name: "folder-music-symbolic"
                        text: i18n("Effects") // qmllint disable
                        checkable: true
                        checked: pageStreamsEffects.streamDB.visiblePage === 1
                        Controls.ActionGroup.group: footerActionGroup
                        onTriggered: {
                            pageStreamsEffects.streamDB.visiblePage = 1;
                        }
                    }
                ]
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                position: Controls.ToolBar.Footer
                overflowIconName: "im-ban-kick-user-symbolic"
                actions: [
                    Kirigami.Action {
                        tooltip: i18n("Input monitoring")// qmllint disable
                        icon.name: "audio-input-microphone-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: pageStreamsEffects.pageType === 1 && pageStreamsEffects.streamDB.visiblePage === 1
                        checkable: true
                        checked: DB.Manager.streamInputs.listenToMic
                        onTriggered: {
                            if (checked !== DB.Manager.streamInputs.listenToMic) {
                                DB.Manager.streamInputs.listenToMic = checked;
                            }
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Excluded Apps")// qmllint disable
                        icon.name: "im-ban-kick-user-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: pageStreamsEffects.streamDB.visiblePage === 0
                        onTriggered: {
                            blocklistSheet.open();
                        }
                    }
                ]
            }
        }
    }
}
