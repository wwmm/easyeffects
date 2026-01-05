/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
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

        parent: pageStreamsEffects.Controls.Overlay.overlay
        streamDB: pageStreamsEffects.streamDB
    }

    BlocklistDialog {
        id: blocklistDialog

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
                    text: i18n("Empty List") // qmllint disable
                    explanation: i18n("No audio application available") // qmllint disable
                    icon.name: "folder-music-symbolic"
                }

                delegate: DelegateStreamsList {}
            }
        }
    }

    Component {
        id: pagePluginsEmptyListMessage

        Kirigami.ScrollablePage {
            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                text: i18n("No Effects") // qmllint disable
                explanation: i18n("Audio stream not modified") // qmllint disable
                icon.name: "folder-music-symbolic"
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
                    [TagsPluginName.BaseName.autogain]: Qt.resolvedUrl("Autogain.qml"),
                    [TagsPluginName.BaseName.bassEnhancer]: Qt.resolvedUrl("BassEnhancer.qml"),
                    [TagsPluginName.BaseName.bassLoudness]: Qt.resolvedUrl("BassLoudness.qml"),
                    [TagsPluginName.BaseName.compressor]: Qt.resolvedUrl("Compressor.qml"),
                    [TagsPluginName.BaseName.convolver]: Qt.resolvedUrl("Convolver.qml"),
                    [TagsPluginName.BaseName.crossfeed]: Qt.resolvedUrl("Crossfeed.qml"),
                    [TagsPluginName.BaseName.crystalizer]: Qt.resolvedUrl("Crystalizer.qml"),
                    [TagsPluginName.BaseName.delay]: Qt.resolvedUrl("Delay.qml"),
                    [TagsPluginName.BaseName.deepfilternet]: Qt.resolvedUrl("DeepFilterNet.qml"),
                    [TagsPluginName.BaseName.deesser]: Qt.resolvedUrl("Deesser.qml"),
                    [TagsPluginName.BaseName.equalizer]: Qt.resolvedUrl("Equalizer.qml"),
                    [TagsPluginName.BaseName.exciter]: Qt.resolvedUrl("Exciter.qml"),
                    [TagsPluginName.BaseName.echoCanceller]: Qt.resolvedUrl("EchoCanceller.qml"),
                    [TagsPluginName.BaseName.expander]: Qt.resolvedUrl("Expander.qml"),
                    [TagsPluginName.BaseName.filter]: Qt.resolvedUrl("Filter.qml"),
                    [TagsPluginName.BaseName.gate]: Qt.resolvedUrl("Gate.qml"),
                    [TagsPluginName.BaseName.levelMeter]: Qt.resolvedUrl("LevelMeter.qml"),
                    [TagsPluginName.BaseName.limiter]: Qt.resolvedUrl("Limiter.qml"),
                    [TagsPluginName.BaseName.loudness]: Qt.resolvedUrl("Loudness.qml"),
                    [TagsPluginName.BaseName.maximizer]: Qt.resolvedUrl("Maximizer.qml"),
                    [TagsPluginName.BaseName.multibandCompressor]: Qt.resolvedUrl("MultibandCompressor.qml"),
                    [TagsPluginName.BaseName.multibandGate]: Qt.resolvedUrl("MultibandGate.qml"),
                    [TagsPluginName.BaseName.pitch]: Qt.resolvedUrl("Pitch.qml"),
                    [TagsPluginName.BaseName.reverb]: Qt.resolvedUrl("Reverb.qml"),
                    [TagsPluginName.BaseName.rnnoise]: Qt.resolvedUrl("RNNoise.qml"),
                    [TagsPluginName.BaseName.speex]: Qt.resolvedUrl("Speex.qml"),
                    [TagsPluginName.BaseName.stereoTools]: Qt.resolvedUrl("StereoTools.qml")
                };

                const packageMap = {
                    [TagsPluginName.BaseName.bassEnhancer]: TagsPluginName.Package.calf,
                    [TagsPluginName.BaseName.bassLoudness]: TagsPluginName.Package.mda,
                    [TagsPluginName.BaseName.compressor]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.crossfeed]: TagsPluginName.Package.bs2b,
                    [TagsPluginName.BaseName.crystalizer]: TagsPluginName.Package.zita,
                    [TagsPluginName.BaseName.delay]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.deepfilternet]: TagsPluginName.Package.deepfilternet,
                    [TagsPluginName.BaseName.deesser]: TagsPluginName.Package.calf,
                    [TagsPluginName.BaseName.equalizer]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.exciter]: TagsPluginName.Package.calf,
                    [TagsPluginName.BaseName.expander]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.filter]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.gate]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.limiter]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.loudness]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.maximizer]: TagsPluginName.Package.zam,
                    [TagsPluginName.BaseName.multibandCompressor]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.multibandGate]: TagsPluginName.Package.lsp,
                    [TagsPluginName.BaseName.reverb]: TagsPluginName.Package.calf,
                    [TagsPluginName.BaseName.rnnoise]: TagsPluginName.Package.rnnoise,
                    [TagsPluginName.BaseName.stereoTools]: TagsPluginName.Package.calf
                };

                const componentUrl = pluginMap[baseName];

                if (!componentUrl) {
                    console.log(`${pageStreamsEffects.logTag} invalid plugin name: ${baseName}`);

                    return;
                }

                const pluginPackage = packageMap[baseName];

                const pluginBackend = pageStreamsEffects.pipelineInstance.getPluginInstance(name);

                if (pluginBackend.packageInstalled === false) {
                    pluginsStack.push(Qt.resolvedUrl("PluginNotAvailable.qml"), {
                        packageName: packageMap[baseName],
                        translatedName: TagsPluginName.PluginsNameModel.translate(baseName)
                    });

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
                        leftDB: pageStreamsEffects.pluginsDB[`${name}#left`],
                        rightDB: pageStreamsEffects.pluginsDB[`${name}#right`]
                    });
                }
            }

            function populatePluginsStack() {
                if (pageStreamsEffects.streamDB.plugins.length < 1) {
                    return;
                }

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

                property var timeDiff: 0
                readonly property real invFps: 1.0 / DbMain.levelMetersFpsCap

                running: pageStreamsEffects.pageType !== 2 && appWindow.visible && pipelineInstance.filtersLinked // qmllint disable

                onTriggered: {
                    if (frameAnimation.timeDiff < invFps) {
                        frameAnimation.timeDiff += frameAnimation.smoothFrameTime;

                        return;
                    }

                    frameAnimation.timeDiff = 0;

                    if (pluginsStack.depth > 1 && pluginsStack.currentItem?.pluginBackend !== undefined)
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
                id: pluginsListLayout

                spacing: 0
                Layout.maximumWidth: DbMain.collapsePluginsList ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 32
                Layout.minimumWidth: DbMain.collapsePluginsList ? Kirigami.Units.gridUnit * 3 : Kirigami.Units.gridUnit * 16
                Layout.preferredWidth: Common.clamp(streamDB.pluginsListWidth, Layout.minimumWidth, Layout.maximumWidth)

                GridLayout {
                    columns: DbMain.collapsePluginsList === true ? 1 : 2
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.mediumSpacing
                    Layout.bottomMargin: Kirigami.Units.mediumSpacing
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                    Layout.alignment: Qt.AlignCenter

                    Controls.Button {
                        text: i18n("Add effect") // qmllint disable
                        display: DbMain.collapsePluginsList === true ? Controls.Button.IconOnly : Controls.Button.TextBesideIcon
                        icon.name: "list-add"
                        onClicked: menuAddPlugins.open()
                        Layout.fillWidth: DbMain.collapsePluginsList === true
                    }

                    Controls.Button {
                        text: i18n("Close") // qmllint disable
                        display: DbMain.collapsePluginsList === true ? Controls.Button.IconOnly : Controls.Button.TextBesideIcon
                        icon.name: DbMain.collapsePluginsList === true ? "sidebar-collapse-right-symbolic" : "sidebar-collapse-symbolic"
                        Layout.fillWidth: DbMain.collapsePluginsList === true
                        Controls.ToolTip.text: DbMain.collapsePluginsList === true ? i18n("Expand the list of effects pipeline") : i18n("Reduce the list of effects pipeline") // qmllint disable
                        Controls.ToolTip.visible: hovered
                        onClicked: {
                            DbMain.collapsePluginsList = !DbMain.collapsePluginsList;
                        }
                    }
                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.smallSpacing
                    Layout.rightMargin: Kirigami.Units.smallSpacing
                }

                ListView {
                    id: pluginsListView

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.bottomMargin: Kirigami.Units.smallSpacing * 2

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

                    header: EeListSectionHeader {
                        width: ListView.view.width
                        visible: pluginsListView.count !== 0

                        icon.name: pageStreamsEffects.pageType === 0 ? "source-playlist-symbolic" : "audio-input-microphone-symbolic"
                        compact: DbMain.collapsePluginsList
                        text: pageStreamsEffects.pageType === 0 ? i18n("Players") : i18n("Input device") // qmllint disable
                    }

                    footer: EeListSectionHeader {
                        width: ListView.view.width
                        visible: pluginsListView.count !== 0

                        icon.name: pageStreamsEffects.pageType === 0 ? "audio-speakers-symbolic" : "source-playlist-symbolic"
                        compact: DbMain.collapsePluginsList
                        text: pageStreamsEffects.pageType === 0 ? i18n("Output device") : i18n("Recorders") // qmllint disable
                    }
                }
            }

            Kirigami.Separator {
                id: pluginsSeparator

                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.preferredWidth: separatorMouseArea.containsMouse ? Kirigami.Units.largeSpacing : implicitWidth

                MouseArea {
                    id: separatorMouseArea

                    property bool dragging: false
                    property real dragStartX: 0
                    property real initialListWidth: 0

                    anchors.fill: parent
                    enabled: !DbMain.collapsePluginsList
                    hoverEnabled: true
                    cursorShape: !DbMain.collapsePluginsList ? Qt.SplitHCursor : Qt.ArrowCursor
                    drag.axis: Drag.XAxis
                    drag.minimumX: pluginsListLayout.Layout.minimumWidth
                    drag.maximumX: pluginsListLayout.Layout.maximumWidth

                    onPressed: mouse => {
                        dragging = true;
                        dragStartX = mouse.x;
                        initialListWidth = pluginsListLayout.width;
                    }

                    onPositionChanged: mouse => {
                        if (dragging && mouse.buttons & Qt.LeftButton) {
                            const deltaX = mouse.x - dragStartX;
                            const newWidth = initialListWidth + deltaX;

                            pluginsListLayout.Layout.preferredWidth = newWidth;
                        }
                    }

                    onReleased: {
                        dragging = false;

                        streamDB.pluginsListWidth = pluginsListLayout.width;
                    }
                }
            }

            Controls.StackView {
                id: pluginsStack

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.horizontalStretchFactor: 3
                visible: pageStreamsEffects.streamDB.visiblePage === 1

                initialItem: pagePluginsEmptyListMessage

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
            implicitHeight: DbSpectrum.height
            seriesType: DbSpectrum.spectrumShape
            colorScheme: DbGraph.colorScheme
            colorTheme: DbGraph.colorTheme
            xMin: DbSpectrum.minimumFrequency
            xMax: DbSpectrum.maximumFrequency
            yMin: -100
            yMax: 0
            logarithimicHorizontalAxis: DbSpectrum.logarithimicHorizontalAxis
            dynamicYScale: DbSpectrum.dynamicYScale
            xUnit: Units.hz
            visible: DbSpectrum.state

            Component.onDestruction: {
                headerFrameAnimation.stop();
            }

            FrameAnimation {
                id: headerFrameAnimation

                property var timeDiff: 0
                readonly property real invFps: 1.0 / DbSpectrum.spectrumFpsCap

                running: DbSpectrum.state && appWindow.visible && pipelineInstance.filtersLinked // qmllint disable

                onRunningChanged: {
                    pageStreamsEffects.pipelineInstance.setSpectrumBypass(!running);
                }

                onTriggered: {
                    if (headerFrameAnimation.timeDiff < invFps) {
                        headerFrameAnimation.timeDiff += headerFrameAnimation.smoothFrameTime;

                        return;
                    }

                    headerFrameAnimation.timeDiff = 0;

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
        position: Controls.ToolBar.Footer

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.smallSpacing
                right: parent.right
                rightMargin: Kirigami.Units.smallSpacing
                bottom: parent.bottom
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

                    property var timeDiff: 0
                    readonly property real invFps: 1.0 / DbMain.levelMetersFpsCap

                    running: pageStreamsEffects.pageType !== 2 && appWindow.visible && pipelineInstance.filtersLinked // qmllint disable

                    onRunningChanged: {
                        pageStreamsEffects.pipelineInstance.setUpdateLevelMeters(running);
                    }

                    onTriggered: {
                        if (footerFrameAnimation.timeDiff < invFps) {
                            footerFrameAnimation.timeDiff += footerFrameAnimation.smoothFrameTime;

                            return;
                        }

                        footerFrameAnimation.timeDiff = 0;

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

                        actionRateValue.text = `<pre ${cssFontColor}> <span ${cssFontWeight}>${rate}</span> ${Units.kHz} </pre>`;
                        actionLatencyValue.text = `<pre ${cssFontColor}> ${styledLatency} ${Units.ms} </pre>`;
                        actionLevelValue.text = `<pre ${cssFontColor}> ${styledLocaleLeft} ${styledLocaleRight} ${Units.dB}</pre>`;
                    }
                }
            }

            RowLayout {
                id: segmentedButton

                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: Math.round(Kirigami.Units.smallSpacing / 2)
                Layout.bottomMargin: Math.round(Kirigami.Units.smallSpacing / 2)

                readonly property bool hasEnoughWidth: appWindow.width >= Kirigami.Units.gridUnit * 40

                readonly property int display: (!Kirigami.Settings.isMobile && hasEnoughWidth) ? Controls.ToolButton.TextBesideIcon : Controls.ToolButton.IconOnly

                Controls.ButtonGroup {
                    id: subNavButtonGroup
                }

                Controls.ToolButton {
                    icon.name: pageStreamsEffects.pageType === 0 ? "multimedia-player-symbolic" : "media-record-symbolic"
                    text: pageStreamsEffects.pageType === 0 ? i18n("Players") : i18n("Recorders") // qmllint disable
                    checkable: true
                    checked: pageStreamsEffects.streamDB.visiblePage === 0
                    display: segmentedButton.display
                    onClicked: {
                        pageStreamsEffects.streamDB.visiblePage = 0;
                    }

                    Controls.ButtonGroup.group: subNavButtonGroup
                }

                Controls.ToolButton {
                    icon.name: "folder-music-symbolic"
                    text: i18n("Effects") // qmllint disable
                    checkable: true
                    checked: pageStreamsEffects.streamDB.visiblePage === 1
                    display: segmentedButton.display
                    onClicked: {
                        pageStreamsEffects.streamDB.visiblePage = 1;
                    }

                    Controls.ButtonGroup.group: subNavButtonGroup
                }
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                position: Controls.ToolBar.Footer
                overflowIconName: "im-ban-kick-user-symbolic"
                actions: [
                    Kirigami.Action {
                        text: i18n("Excluded apps") // qmllint disable
                        icon.name: "im-ban-kick-user-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: pageStreamsEffects.streamDB.visiblePage === 0
                        onTriggered: {
                            blocklistDialog.open();
                        }
                    },
                    Kirigami.Action {
                        tooltip: i18n("Input monitoring") // qmllint disable
                        icon.name: "audio-input-microphone-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: pageStreamsEffects.pageType === 1
                        checkable: true
                        checked: DbStreamInputs.listenToMic
                        onTriggered: {
                            if (checked !== DbStreamInputs.listenToMic) {
                                DbStreamInputs.listenToMic = checked;
                            }
                        }
                    },
                    Kirigami.Action {
                        tooltip: i18n("Send audio to the virtual source") // qmllint disable
                        icon.name: "emblem-shared-symbolic"
                        displayHint: Kirigami.DisplayHint.KeepVisible
                        visible: pageStreamsEffects.pageType === 0
                        checkable: true
                        checked: DbStreamOutputs.linkToVirtualSource
                        onTriggered: {
                            if (checked !== DbStreamOutputs.linkToVirtualSource) {
                                DbStreamOutputs.linkToVirtualSource = checked;
                            }
                        }
                    }
                ]
            }
        }
    }
}
