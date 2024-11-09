import "Common.js" as Common
import EEdbm
import EEpw
import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ScrollablePage {
    id: compressorPage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return ;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
        reductionLevelLeft.value = pluginBackend.getReductionLevelLeft();
        reductionLevelRight.value = pluginBackend.getReductionLevelRight();
        sideChainLevelLeft.value = pluginBackend.getSideChainLevelLeft();
        sideChainLevelRight.value = pluginBackend.getSideChainLevelRight();
        curveLevelLeft.value = pluginBackend.getCurveLevelLeft();
        curveLevelRight.value = pluginBackend.getCurveLevelRight();
        envelopeLevelLeft.value = pluginBackend.getEnvelopeLevelLeft();
        envelopeLevelRight.value = pluginBackend.getEnvelopeLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            maximumColumns: 2
            uniformCellWidths: true

            Kirigami.Card {
                id: cardLimiter

                header: Kirigami.Heading {
                    text: i18n("Compressor")
                    level: 2
                }

                contentItem: Column {
                    EeSpinBox {
                        id: threshold

                        label: i18n("Threshold")
                        from: -48
                        to: 0
                        value: pluginDB.threshold
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.threshold = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: attack

                        label: i18n("Attack")
                        from: 0.25
                        to: 20
                        value: pluginDB.attack
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.attack = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: release

                        label: i18n("Release")
                        from: 0.25
                        to: 20
                        value: pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.release = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: stereoLink

                        label: i18n("Stereo Link")
                        from: 0
                        to: 100
                        value: pluginDB.stereoLink
                        decimals: 1
                        stepSize: 0.1
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.stereoLink = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                }

            }

            Kirigami.Card {
                id: cardSideChain

                header: Kirigami.Heading {
                    text: i18n("Sidechain")
                    level: 2
                }

                contentItem: Column {
                    id: cardSideChainColumn

                    FormCard.FormComboBoxDelegate {
                        id: sidechainType

                        text: i18n("Type")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.sidechainType
                        editable: false
                        model: [i18n("Feed-forward"), i18n("Feed-back"), i18n("External"), i18n("Link")]
                        onActivated: (idx) => {
                            pluginDB.sidechainType = idx;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    FormCard.FormComboBoxDelegate {
                        id: sidechainMode

                        text: i18n("Mode")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.sidechainMode
                        editable: false
                        model: [i18n("Peak"), i18n("RMS"), i18n("Low-Pass"), i18n("SMA")]
                        onActivated: (idx) => {
                            pluginDB.sidechainMode = idx;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    FormCard.FormComboBoxDelegate {
                        readonly property var sourceModel: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]
                        readonly property var stereSplitModel: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]

                        text: i18n("Source")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        // currentIndex: pluginDB.stereoSplit === false ? pluginDB.sidechainSource : pluginDB.stereoSplitSource
                        editable: false
                        model: pluginDB.stereoSplit === false ? sourceModel : stereSplitModel
                        onActivated: (idx) => {
                            if (pluginDB.stereoSplit)
                                pluginDB.stereoSplitSource = idx;
                            else
                                pluginDB.sidechainSource = idx;
                        }
                        onModelChanged: {
                            if (pluginDB.stereoSplit)
                                currentIndex = Qt.binding(function() {
                                return pluginDB.stereoSplitSource;
                            });
                            else
                                currentIndex = Qt.binding(function() {
                                return pluginDB.sidechainSource;
                            });
                            console.log("model: " + currentIndex);
                        }
                        onCurrentIndexChanged: {
                            console.log(currentValue);
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    FormCard.FormComboBoxDelegate {
                        id: comboSideChainInputDevice

                        text: i18n("Input Device")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        editable: false
                        model: ModelNodes
                        textRole: "description"
                        enabled: sidechainType.currentIndex === 2
                        currentIndex: {
                            for (let n = 0; n < ModelNodes.rowCount(); n++) {
                                if (ModelNodes.getNodeName(n) === pluginDB.sidechainInputDevice)
                                    return n;

                            }
                            return 0;
                        }
                        onActivated: (idx) => {
                            let selectedName = ModelNodes.getNodeName(idx);
                            if (selectedName !== pluginDB.sidechainInputDevice)
                                pluginDB.sidechainInputDevice = selectedName;

                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: sidechainPreamp

                        label: i18n("SC Preamp")
                        from: -80.01
                        to: 40
                        value: pluginDB.sidechainPreamp
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: (v) => {
                            pluginDB.sidechainPreamp = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                }

            }

        }

        Kirigami.CardsLayout {
            id: cardLayoutLevel

            maximumColumns: 2
            uniformCellWidths: true

            Kirigami.Card {
                id: cardLevels1

                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter

                contentItem: GridLayout {
                    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Reduction")
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Sidechain")
                    }

                    EeCircularProgress {
                        id: reductionLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: reductionLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: sideChainLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: sideChainLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                }

            }

            Kirigami.Card {
                id: cardLevels2

                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter

                contentItem: GridLayout {
                    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

                    columnSpacing: Kirigami.Units.largeSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Curve")
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Envelope")
                    }

                    EeCircularProgress {
                        id: curveLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: curveLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: envelopeLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeCircularProgress {
                        id: envelopeLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: compressorPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.lsp
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
            Layout.leftMargin: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.disabledTextColor
        }

        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    text: i18n("Show Native Window")
                    icon.name: "window-duplicate-symbolic"
                    enabled: EEdbm.main.showNativePluginUi
                    checkable: true
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.show_native_ui();
                        else
                            pluginBackend.close_native_ui();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen")
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: pluginDB.sidechainListen
                    onTriggered: {
                        if (pluginDB.sidechainListen != checked)
                            pluginDB.sidechainListen = checked;

                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo Split")
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: pluginDB.stereoSplit
                    onTriggered: {
                        if (pluginDB.stereoSplit != checked)
                            pluginDB.stereoSplit = checked;

                    }
                },
                Kirigami.Action {
                    text: i18n("Reset")
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        pluginBackend.reset();
                    }
                }
            ]
        }

    }

}
