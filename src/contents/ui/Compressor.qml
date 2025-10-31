import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigami.layouts as KirigamiLayouts
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: compressorPage

    required property string name
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

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            maximumColumns: 6
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Compressor") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            Layout.columnSpan: 2
                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.mode
                            editable: false
                            model: [i18n("Downward"), i18n("Upward"), i18n("Boosting")] //qmllint disable
                            onActivated: idx => {
                                compressorPage.pluginDB.mode = idx;
                            }
                        }

                        EeSpinBox {
                            id: boostThreshold

                            Layout.columnSpan: 2
                            label: i18n("Boost threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("boostThreshold")
                            to: compressorPage.pluginDB.getMaxValue("boostThreshold")
                            value: compressorPage.pluginDB.boostThreshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: mode.currentIndex === 1
                            visible: mode.currentIndex === 1
                            onValueModified: v => {
                                compressorPage.pluginDB.boostThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: boostAmount

                            Layout.columnSpan: 2
                            label: i18n("Boost amount") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("boostAmount")
                            to: compressorPage.pluginDB.getMaxValue("boostAmount")
                            value: compressorPage.pluginDB.boostAmount
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: mode.currentIndex === 2
                            visible: mode.currentIndex === 2
                            onValueModified: v => {
                                compressorPage.pluginDB.boostAmount = v;
                            }
                        }

                        EeSpinBox {
                            id: ratio

                            label: i18n("Ratio") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("ratio")
                            to: compressorPage.pluginDB.getMaxValue("ratio")
                            value: compressorPage.pluginDB.ratio
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                compressorPage.pluginDB.ratio = v;
                            }
                        }

                        EeSpinBox {
                            id: knee

                            label: i18n("Knee") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("knee")
                            to: compressorPage.pluginDB.getMaxValue("knee")
                            value: compressorPage.pluginDB.knee
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                compressorPage.pluginDB.knee = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Threshold and Time") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: threshold

                            label: i18n("Attack") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("threshold")
                            to: compressorPage.pluginDB.getMaxValue("threshold")
                            value: compressorPage.pluginDB.threshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                compressorPage.pluginDB.threshold = v;
                            }
                        }

                        EeSpinBox {
                            id: attack

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("attack")
                            to: compressorPage.pluginDB.getMaxValue("attack")
                            value: compressorPage.pluginDB.attack
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                compressorPage.pluginDB.attack = v;
                            }
                        }

                        EeSpinBox {
                            id: releaseThreshold

                            label: i18n("Release") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("releaseThreshold")
                            to: compressorPage.pluginDB.getMaxValue("releaseThreshold")
                            value: compressorPage.pluginDB.releaseThreshold
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.releaseThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: release

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("release")
                            to: compressorPage.pluginDB.getMaxValue("release")
                            value: compressorPage.pluginDB.release
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                compressorPage.pluginDB.release = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Sidechain") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: sidechainType

                            Layout.columnSpan: 2
                            text: i18n("Type") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.sidechainType
                            editable: false
                            model: [i18n("Feed-forward"), i18n("Feed-back"), i18n("External"), i18n("Link")] // qmllint disable
                            onActivated: idx => {
                                compressorPage.pluginDB.sidechainType = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: comboSideChainInputDevice

                            Layout.columnSpan: 2
                            text: i18n("Input device") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            editable: false
                            model: PW.ModelNodes
                            textRole: "description"
                            enabled: sidechainType.currentIndex === 2
                            currentIndex: {
                                for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                    if (PW.ModelNodes.getNodeName(n) === compressorPage.pluginDB.sidechainInputDevice)
                                        return n;
                                }
                                return 0;
                            }
                            onActivated: idx => {
                                let selectedName = PW.ModelNodes.getNodeName(idx);
                                if (selectedName !== compressorPage.pluginDB.sidechainInputDevice)
                                    compressorPage.pluginDB.sidechainInputDevice = selectedName;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainMode

                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.sidechainMode
                            editable: false
                            model: [i18n("Peak"), i18n("RMS"), i18n("Low-pass"), i18n("SMA")] // qmllint disable
                            onActivated: idx => {
                                compressorPage.pluginDB.sidechainMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.sidechainSource
                            editable: false
                            model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: !compressorPage.pluginDB.stereoSplit
                            onActivated: idx => {
                                compressorPage.pluginDB.sidechainSource = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: stereoSplitSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.stereoSplitSource
                            editable: false
                            model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: compressorPage.pluginDB.stereoSplit
                            onActivated: idx => {
                                compressorPage.pluginDB.stereoSplitSource = idx;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Sidechain") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: sidechainPreamp

                            label: i18n("Preamp") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("sidechainPreamp")
                            to: compressorPage.pluginDB.getMaxValue("sidechainPreamp")
                            value: compressorPage.pluginDB.sidechainPreamp
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.sidechainPreamp = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainReactivity

                            label: i18n("Reactivity") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("sidechainReactivity")
                            to: compressorPage.pluginDB.getMaxValue("sidechainReactivity")
                            value: compressorPage.pluginDB.sidechainReactivity
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                compressorPage.pluginDB.sidechainReactivity = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainLookahead

                            Layout.columnSpan: 2
                            label: i18n("Lookahead") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("sidechainLookahead")
                            to: compressorPage.pluginDB.getMaxValue("sidechainLookahead")
                            value: compressorPage.pluginDB.sidechainLookahead
                            decimals: 3
                            stepSize: 0.001
                            unit: "ms"
                            onValueModified: v => {
                                compressorPage.pluginDB.sidechainLookahead = v;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: hpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("High-pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.hpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")] // qmllint disable
                            onActivated: idx => {
                                compressorPage.pluginDB.hpfMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: lpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("Low-pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: compressorPage.pluginDB.lpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]// qmllint disable
                            onActivated: idx => {
                                compressorPage.pluginDB.lpfMode = idx;
                            }
                        }

                        EeSpinBox {
                            id: hpfFrequency

                            horizontalPadding: 0
                            verticalPadding: 0
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("hpfFrequency")
                            to: compressorPage.pluginDB.getMaxValue("hpfFrequency")
                            value: compressorPage.pluginDB.hpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: hpfMode.currentIndex !== 0
                            onValueModified: v => {
                                compressorPage.pluginDB.hpfFrequency = v;
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
                            from: compressorPage.pluginDB.getMinValue("lpfFrequency")
                            to: compressorPage.pluginDB.getMaxValue("lpfFrequency")
                            value: compressorPage.pluginDB.lpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: lpfMode.currentIndex !== 0
                            onValueModified: v => {
                                compressorPage.pluginDB.lpfFrequency = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Output") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSpinBox {
                        id: dry

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: compressorPage.pluginDB.getMinValue("dry")
                        to: compressorPage.pluginDB.getMaxValue("dry")
                        value: compressorPage.pluginDB.dry
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            compressorPage.pluginDB.dry = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: compressorPage.pluginDB.getMinValue("wet")
                        to: compressorPage.pluginDB.getMaxValue("wet")
                        value: compressorPage.pluginDB.wet
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            compressorPage.pluginDB.wet = v;
                        }
                    }

                    EeSpinBox {
                        id: makeup

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Makeup") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: compressorPage.pluginDB.getMinValue("makeup")
                        to: compressorPage.pluginDB.getMaxValue("makeup")
                        value: compressorPage.pluginDB.makeup
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            compressorPage.pluginDB.makeup = v;
                        }
                    }
                }
            }

            Kirigami.Card {
                header: Kirigami.Heading {
                    text: i18n("Pre-Mix") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            label: i18n("Input to sidechain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("inputToSidechain")
                            to: compressorPage.pluginDB.getMaxValue("inputToSidechain")
                            value: compressorPage.pluginDB.inputToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.inputToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Input to link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("inputToLink")
                            to: compressorPage.pluginDB.getMaxValue("inputToLink")
                            value: compressorPage.pluginDB.inputToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.inputToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("sidechainToInput")
                            to: compressorPage.pluginDB.getMaxValue("sidechainToInput")
                            value: compressorPage.pluginDB.sidechainToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.sidechainToInput = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("sidechainToLink")
                            to: compressorPage.pluginDB.getMaxValue("sidechainToLink")
                            value: compressorPage.pluginDB.sidechainToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.sidechainToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to sidechain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("linkToSidechain")
                            to: compressorPage.pluginDB.getMaxValue("linkToSidechain")
                            value: compressorPage.pluginDB.linkToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.linkToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: compressorPage.pluginDB.getMinValue("linkToInput")
                            to: compressorPage.pluginDB.getMaxValue("linkToInput")
                            value: compressorPage.pluginDB.linkToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                compressorPage.pluginDB.linkToInput = v;
                            }
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
                        text: i18n("Reduction") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Sidechain") // qmllint disable
                    }

                    EeAudioLevel {
                        id: reductionLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 72
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                        topToBottom: true
                    }

                    EeAudioLevel {
                        id: reductionLevelRight

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 72
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                        topToBottom: true
                    }

                    EeAudioLevel {
                        id: sideChainLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
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
                        to: 36
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R") // qmllint disable
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
                        text: i18n("Curve") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Envelope") // qmllint disable
                    }

                    EeAudioLevel {
                        id: curveLevelLeft

                        Layout.alignment: Qt.AlignBottom
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 36
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
                        to: 36
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
                        to: 36
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
                        to: 36
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("L") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("R") // qmllint disable
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</strong>`) // qmllint disable
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
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: compressorPage.pluginBackend ? compressorPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            compressorPage.pluginBackend.showNativeUi();
                        else
                            compressorPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: compressorPage.pluginDB.sidechainListen
                    onTriggered: {
                        if (compressorPage.pluginDB.sidechainListen != checked)
                            compressorPage.pluginDB.sidechainListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: compressorPage.pluginDB.stereoSplit
                    onTriggered: {
                        if (compressorPage.pluginDB.stereoSplit != checked)
                            compressorPage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        compressorPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
