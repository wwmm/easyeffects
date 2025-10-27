import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigami.layouts as KirigamiLayouts
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: gatePage

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
        attackZoneStart.value = pluginBackend.getAttackZoneStart();
        attackThreshold.value = pluginBackend.getAttackThreshold();
        releaseZoneStart.value = pluginBackend.getReleaseZoneStart();
        releaseThreshold.value = pluginBackend.getReleaseThreshold();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            maximumColumns: 6
            minimumColumnWidth: Kirigami.Units.gridUnit * 16
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Gate") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: attack

                            label: i18n("Attack") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("attack")
                            to: gatePage.pluginDB.getMaxValue("attack")
                            value: gatePage.pluginDB.attack
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                gatePage.pluginDB.attack = v;
                            }
                        }

                        EeSpinBox {
                            id: release

                            label: i18n("Release") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("release")
                            to: gatePage.pluginDB.getMaxValue("release")
                            value: gatePage.pluginDB.release
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                gatePage.pluginDB.release = v;
                            }
                        }

                        EeSpinBox {
                            id: reduction

                            Layout.columnSpan: 2
                            label: i18n("Reduction") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("reduction")
                            to: gatePage.pluginDB.getMaxValue("reduction")
                            value: gatePage.pluginDB.reduction
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                gatePage.pluginDB.reduction = v;
                            }
                        }

                        EeSpinBox {
                            id: curveThreshold

                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("curveThreshold")
                            to: gatePage.pluginDB.getMaxValue("curveThreshold")
                            value: gatePage.pluginDB.curveThreshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                gatePage.pluginDB.curveThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: curveZone

                            label: i18n("Zone") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("curveZone")
                            to: gatePage.pluginDB.getMaxValue("curveZone")
                            value: gatePage.pluginDB.curveZone
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                gatePage.pluginDB.curveZone = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Hysteresis") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent
                    Layout.alignment: Qt.AlignTop

                    EeSwitch {
                        id: hysteresis

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Enable") // qmllint disable
                        isChecked: gatePage.pluginDB.hysteresis
                        onCheckedChanged: {
                            if (isChecked !== gatePage.pluginDB.hysteresis)
                                gatePage.pluginDB.hysteresis = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: hysteresisThreshold

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Threshold") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("hysteresisThreshold")
                        to: gatePage.pluginDB.getMaxValue("hysteresisThreshold")
                        value: gatePage.pluginDB.hysteresisThreshold
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        enabled: hysteresis.isChecked
                        onValueModified: v => {
                            gatePage.pluginDB.hysteresisThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: hysteresisZone

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Zone") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("hysteresisZone")
                        to: gatePage.pluginDB.getMaxValue("hysteresisZone")
                        value: gatePage.pluginDB.hysteresisZone
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        enabled: hysteresis.isChecked
                        onValueModified: v => {
                            gatePage.pluginDB.hysteresisZone = v;
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
                            currentIndex: gatePage.pluginDB.sidechainType
                            editable: false
                            model: [i18n("Internal"), i18n("External"), i18n("Link")]// qmllint disable
                            onActivated: idx => {
                                gatePage.pluginDB.sidechainType = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: comboSideChainInputDevice

                            Layout.columnSpan: 2
                            text: i18n("Input Device") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            editable: false
                            model: PW.ModelNodes
                            textRole: "description"
                            enabled: sidechainType.currentIndex === 1
                            currentIndex: {
                                for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                    if (PW.ModelNodes.getNodeName(n) === gatePage.pluginDB.sidechainInputDevice)
                                        return n;
                                }
                                return 0;
                            }
                            onActivated: idx => {
                                let selectedName = PW.ModelNodes.getNodeName(idx);
                                if (selectedName !== gatePage.pluginDB.sidechainInputDevice)
                                    gatePage.pluginDB.sidechainInputDevice = selectedName;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainMode

                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: gatePage.pluginDB.sidechainMode
                            editable: false
                            model: [i18n("Peak"), i18n("RMS"), i18n("Low-Pass"), i18n("SMA")]// qmllint disable
                            onActivated: idx => {
                                gatePage.pluginDB.sidechainMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: gatePage.pluginDB.sidechainSource
                            editable: false
                            model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: !gatePage.pluginDB.stereoSplit
                            onActivated: idx => {
                                gatePage.pluginDB.sidechainSource = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: stereoSplitSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: gatePage.pluginDB.stereoSplitSource
                            editable: false
                            model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: gatePage.pluginDB.stereoSplit
                            onActivated: idx => {
                                gatePage.pluginDB.stereoSplitSource = idx;
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
                            from: gatePage.pluginDB.getMinValue("sidechainPreamp")
                            to: gatePage.pluginDB.getMaxValue("sidechainPreamp")
                            value: gatePage.pluginDB.sidechainPreamp
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.sidechainPreamp = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainReactivity

                            label: i18n("Reactivity") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("sidechainReactivity")
                            to: gatePage.pluginDB.getMaxValue("sidechainReactivity")
                            value: gatePage.pluginDB.sidechainReactivity
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                gatePage.pluginDB.sidechainReactivity = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainLookahead

                            Layout.columnSpan: 2
                            label: i18n("Lookahead") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("sidechainLookahead")
                            to: gatePage.pluginDB.getMaxValue("sidechainLookahead")
                            value: gatePage.pluginDB.sidechainLookahead
                            decimals: 3
                            stepSize: 0.001
                            unit: "ms"
                            onValueModified: v => {
                                gatePage.pluginDB.sidechainLookahead = v;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: hpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("High-Pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: gatePage.pluginDB.hpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]// qmllint disable
                            onActivated: idx => {
                                gatePage.pluginDB.hpfMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: lpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("Low-Pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: gatePage.pluginDB.lpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]// qmllint disable
                            onActivated: idx => {
                                gatePage.pluginDB.lpfMode = idx;
                            }
                        }

                        EeSpinBox {
                            id: hpfFrequency

                            horizontalPadding: 0
                            verticalPadding: 0
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("hpfFrequency")
                            to: gatePage.pluginDB.getMaxValue("hpfFrequency")
                            value: gatePage.pluginDB.hpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: hpfMode.currentIndex !== 0
                            onValueModified: v => {
                                gatePage.pluginDB.hpfFrequency = v;
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
                            from: gatePage.pluginDB.getMinValue("lpfFrequency")
                            to: gatePage.pluginDB.getMaxValue("lpfFrequency")
                            value: gatePage.pluginDB.lpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: lpfMode.currentIndex !== 0
                            onValueModified: v => {
                                gatePage.pluginDB.lpfFrequency = v;
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
                    Layout.alignment: Qt.AlignTop

                    EeSpinBox {
                        id: dry

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("dry")
                        to: gatePage.pluginDB.getMaxValue("dry")
                        value: gatePage.pluginDB.dry
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.dry = v;
                        }
                    }

                    EeSpinBox {
                        id: wet

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("wet")
                        to: gatePage.pluginDB.getMaxValue("wet")
                        value: gatePage.pluginDB.wet
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            gatePage.pluginDB.wet = v;
                        }
                    }

                    EeSpinBox {
                        id: makeup

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Makeup") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: gatePage.pluginDB.getMinValue("makeup")
                        to: gatePage.pluginDB.getMaxValue("makeup")
                        value: gatePage.pluginDB.makeup
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            gatePage.pluginDB.makeup = v;
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
                            label: i18n("Input to Sidechain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("inputToSidechain")
                            to: gatePage.pluginDB.getMaxValue("inputToSidechain")
                            value: gatePage.pluginDB.inputToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.inputToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Input to Link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("inputToLink")
                            to: gatePage.pluginDB.getMaxValue("inputToLink")
                            value: gatePage.pluginDB.inputToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.inputToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to Input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("sidechainToInput")
                            to: gatePage.pluginDB.getMaxValue("sidechainToInput")
                            value: gatePage.pluginDB.sidechainToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.sidechainToInput = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to Link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("sidechainToLink")
                            to: gatePage.pluginDB.getMaxValue("sidechainToLink")
                            value: gatePage.pluginDB.sidechainToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.sidechainToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to Sidechain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("linkToSidechain")
                            to: gatePage.pluginDB.getMaxValue("linkToSidechain")
                            value: gatePage.pluginDB.linkToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.linkToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to Input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: gatePage.pluginDB.getMinValue("linkToInput")
                            to: gatePage.pluginDB.getMaxValue("linkToInput")
                            value: gatePage.pluginDB.linkToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                gatePage.pluginDB.linkToInput = v;
                            }
                        }
                    }
                }
            }
        }

        Kirigami.CardsLayout {
            maximumColumns: 3
            minimumColumnWidth: Kirigami.Units.gridUnit * 17
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
                        to: 0
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
                        to: 0
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
                        to: 24
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
                        to: 24
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
                        to: 24
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
                        to: 24
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
                        to: 24
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
                        to: 24
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

                    columnSpacing: Kirigami.Units.smallSpacing
                    rowSpacing: Kirigami.Units.largeSpacing
                    columns: 4
                    rows: 3

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Attack") // qmllint disable
                    }

                    Controls.Label {
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        topPadding: Kirigami.Units.smallSpacing
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Release") // qmllint disable
                    }

                    EeAudioLevel {
                        id: attackZoneStart

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeAudioLevel {
                        id: attackThreshold

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
                        implicitWidth: parent.radius
                        implicitHeight: parent.radius
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 0
                        convertDecibelToLinear: true
                    }

                    EeAudioLevel {
                        id: releaseZoneStart

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
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
                        id: releaseThreshold

                        Layout.alignment: Qt.AlignBottom | Text.AlignHCenter
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
                        text: i18n("Start") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Threshold") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Start") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        text: i18n("Threshold") // qmllint disable
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: gatePage.pluginDB
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
                    text: i18n("Show Native Window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: gatePage.pluginBackend ? gatePage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            gatePage.pluginBackend.showNativeUi();
                        else
                            gatePage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: gatePage.pluginDB.sidechainListen
                    onTriggered: {
                        if (gatePage.pluginDB.sidechainListen != checked)
                            gatePage.pluginDB.sidechainListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo Split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: gatePage.pluginDB.stereoSplit
                    onTriggered: {
                        if (gatePage.pluginDB.stereoSplit != checked)
                            gatePage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        gatePage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
