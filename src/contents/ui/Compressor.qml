import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: compressorPage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

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

            maximumColumns: 5
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Compressor")
                    level: 2
                }

                contentItem: Column {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.mode
                            editable: false
                            model: [i18n("Downward"), i18n("Upward"), i18n("Boosting")]
                            onActivated: idx => {
                                pluginDB.mode = idx;
                            }
                        }

                        EeSpinBox {
                            id: boostThreshold

                            Layout.columnSpan: 2
                            label: i18n("Boost Threshold")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("boostThreshold")
                            to: pluginDB.getMaxValue("boostThreshold")
                            value: pluginDB.boostThreshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: mode.currentIndex === 1
                            visible: mode.currentIndex === 1
                            onValueModified: v => {
                                pluginDB.boostThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: boostAmount

                            Layout.columnSpan: 2
                            label: i18n("Boost Amount")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("boostAmount")
                            to: pluginDB.getMaxValue("boostAmount")
                            value: pluginDB.boostAmount
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: mode.currentIndex === 2
                            visible: mode.currentIndex === 2
                            onValueModified: v => {
                                pluginDB.boostAmount = v;
                            }
                        }

                        EeSpinBox {
                            id: ratio

                            label: i18n("Ratio")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("ratio")
                            to: pluginDB.getMaxValue("ratio")
                            value: pluginDB.ratio
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pluginDB.ratio = v;
                            }
                        }

                        EeSpinBox {
                            id: knee

                            label: i18n("Knee")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("knee")
                            to: pluginDB.getMaxValue("knee")
                            value: pluginDB.knee
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB.knee = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Threshold and Time")
                    level: 2
                }

                contentItem: Column {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        EeSpinBox {
                            id: threshold

                            label: i18n("Attack")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("threshold")
                            to: pluginDB.getMaxValue("threshold")
                            value: pluginDB.threshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB.threshold = v;
                            }
                        }

                        EeSpinBox {
                            id: attack

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("attack")
                            to: pluginDB.getMaxValue("attack")
                            value: pluginDB.attack
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.attack = v;
                            }
                        }

                        EeSpinBox {
                            id: releaseThreshold

                            label: i18n("Release")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("releaseThreshold")
                            to: pluginDB.getMaxValue("releaseThreshold")
                            value: pluginDB.releaseThreshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.releaseThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: release

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("release")
                            to: pluginDB.getMaxValue("release")
                            value: pluginDB.release
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.release = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Sidechain")
                    level: 2
                }

                contentItem: Column {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainType

                            Layout.columnSpan: 2
                            text: i18n("Type")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.sidechainType
                            editable: false
                            model: [i18n("Feed-forward"), i18n("Feed-back"), i18n("External"), i18n("Link")]
                            onActivated: idx => {
                                pluginDB.sidechainType = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: comboSideChainInputDevice

                            Layout.columnSpan: 2
                            text: i18n("Input Device")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            editable: false
                            model: PW.ModelNodes
                            textRole: "description"
                            enabled: sidechainType.currentIndex === 2
                            currentIndex: {
                                for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                    if (PW.ModelNodes.getNodeName(n) === pluginDB.sidechainInputDevice)
                                        return n;
                                }
                                return 0;
                            }
                            onActivated: idx => {
                                let selectedName = PW.ModelNodes.getNodeName(idx);
                                if (selectedName !== pluginDB.sidechainInputDevice)
                                    pluginDB.sidechainInputDevice = selectedName;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainMode

                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.sidechainMode
                            editable: false
                            model: [i18n("Peak"), i18n("RMS"), i18n("Low-Pass"), i18n("SMA")]
                            onActivated: idx => {
                                pluginDB.sidechainMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainSource

                            text: i18n("Source")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.sidechainSource
                            editable: false
                            model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]
                            visible: !pluginDB.stereoSplit
                            onActivated: idx => {
                                pluginDB.sidechainSource = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: stereoSplitSource

                            text: i18n("Source")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.stereoSplitSource
                            editable: false
                            model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]
                            visible: pluginDB.stereoSplit
                            onActivated: idx => {
                                pluginDB.stereoSplitSource = idx;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Sidechain")
                    level: 2
                }

                contentItem: Column {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        EeSpinBox {
                            id: sidechainPreamp

                            label: i18n("Preamp")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sidechainPreamp")
                            to: pluginDB.getMaxValue("sidechainPreamp")
                            value: pluginDB.sidechainPreamp
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.sidechainPreamp = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainReactivity

                            label: i18n("Reactivity")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sidechainReactivity")
                            to: pluginDB.getMaxValue("sidechainReactivity")
                            value: pluginDB.sidechainReactivity
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.sidechainReactivity = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainLookahead

                            Layout.columnSpan: 2
                            label: i18n("Lookahead")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sidechainLookahead")
                            to: pluginDB.getMaxValue("sidechainLookahead")
                            value: pluginDB.sidechainLookahead
                            decimals: 3
                            stepSize: 0.001
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.sidechainLookahead = v;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: hpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("High-Pass")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.hpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]
                            onActivated: idx => {
                                pluginDB.hpfMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: lpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("Low-Pass")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.lpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]
                            onActivated: idx => {
                                pluginDB.lpfMode = idx;
                            }
                        }

                        EeSpinBox {
                            id: hpfFrequency

                            horizontalPadding: 0
                            verticalPadding: 0
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("hpfFrequency")
                            to: pluginDB.getMaxValue("hpfFrequency")
                            value: pluginDB.hpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: hpfMode.currentIndex !== 0
                            onValueModified: v => {
                                pluginDB.hpfFrequency = v;
                            }
                        }

                        EeSpinBox {
                            id: lpfFrequency

                            Layout.column: 1
                            Layout.row: 3
                            horizontalPadding: 0
                            verticalPadding: 0
                            labelFillWidth: false
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("lpfFrequency")
                            to: pluginDB.getMaxValue("lpfFrequency")
                            value: pluginDB.lpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: lpfMode.currentIndex !== 0
                            onValueModified: v => {
                                pluginDB.lpfFrequency = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Output")
                    level: 2
                }

                contentItem: Column {
                    EeSpinBox {
                        id: dry

                        label: i18n("Dry")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("dry")
                        to: pluginDB.getMaxValue("dry")
                        value: pluginDB.dry
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            pluginDB.dry = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("wet")
                        to: pluginDB.getMaxValue("wet")
                        value: pluginDB.wet
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            pluginDB.wet = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: makeup

                        Layout.columnSpan: 2
                        label: i18n("Makeup")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("makeup")
                        to: pluginDB.getMaxValue("makeup")
                        value: pluginDB.makeup
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.makeup = v;
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
            maximumColumns: 2
            uniformCellWidths: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            Kirigami.Card {
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
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Reduction")
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Sidechain")
                    }

                    EeAudioLevel {
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

                    EeAudioLevel {
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

                    EeAudioLevel {
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

                    EeAudioLevel {
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
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }
                }
            }

            Kirigami.Card {
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
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Curve")
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Envelope")
                    }

                    EeAudioLevel {
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

                    EeAudioLevel {
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

                    EeAudioLevel {
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

                    EeAudioLevel {
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
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.lsp}</b>`)
            textFormat: Text.RichText
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
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.showNativeUi();
                        else
                            pluginBackend.closeNativeUi();
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
