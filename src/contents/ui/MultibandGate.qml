pragma ComponentBehavior: Bound
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
    id: multibandGatePage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend
    readonly property string bandId: "band" + bandsListview.currentIndex
    property list<real> bandFrequencyEnd: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandReductionLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandReductionLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandEnvelopeLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandEnvelopeLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandCurveLevelLeft: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    property list<real> bandCurveLevelRight: [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
        bandFrequencyEnd = pluginBackend.getFrequencyRangeEnd();
        bandReductionLevelLeft = pluginBackend.getReductionLevelLeft();
        bandReductionLevelRight = pluginBackend.getReductionLevelRight();
        bandEnvelopeLevelLeft = pluginBackend.getEnvelopeLevelLeft();
        bandEnvelopeLevelRight = pluginBackend.getEnvelopeLevelRight();
        bandCurveLevelLeft = pluginBackend.getCurveLevelLeft();
        bandCurveLevelRight = pluginBackend.getCurveLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    Component {
        id: bandGateControls

        Column {
            spacing: Kirigami.Units.gridUnit

            Kirigami.ActionToolBar {
                Layout.margins: Kirigami.Units.smallSpacing
                alignment: Qt.AlignHCenter
                position: Controls.ToolBar.Header
                flat: false

                anchors {
                    left: parent.left
                    right: parent.right
                }

                actions: [
                    Kirigami.Action {
                        text: i18n("Mute")
                        checkable: true
                        checked: pluginDB[multibandGatePage.bandId + "Mute"]
                        onTriggered: {
                            if (pluginDB[multibandGatePage.bandId + "Mute"] != checked)
                                pluginDB[multibandGatePage.bandId + "Mute"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Solo")
                        checkable: true
                        checked: pluginDB[multibandGatePage.bandId + "Solo"]
                        onTriggered: {
                            if (pluginDB[multibandGatePage.bandId + "Solo"] != checked)
                                pluginDB[multibandGatePage.bandId + "Solo"] = checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Bypass")
                        checkable: true
                        checked: !pluginDB[multibandGatePage.bandId + "GateEnable"]
                        onTriggered: {
                            pluginDB[multibandGatePage.bandId + "GateEnable"] = !checked;
                        }
                    }
                ]
            }

            Kirigami.CardsLayout {
                maximumColumns: 5
                minimumColumnWidth: Kirigami.Units.gridUnit * 15
                uniformCellWidths: true

                anchors {
                    left: parent.left
                    right: parent.right
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Frequency")
                        }

                        EeSpinBox {
                            label: i18n("Start")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: bandsListview.currentIndex > 0 ? pluginDB.getMinValue(multibandGatePage.bandId + "SplitFrequency") : from
                            to: bandsListview.currentIndex > 0 ? pluginDB.getMaxValue(multibandGatePage.bandId + "SplitFrequency") : to
                            value: bandsListview.currentIndex > 0 ? pluginDB[multibandGatePage.bandId + "SplitFrequency"] : 0
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            enabled: bandsListview.currentIndex > 0
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "SplitFrequency"] = v;
                            }
                        }

                        ColumnLayout {
                            Controls.Label {
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("End")
                            }

                            Controls.Label {
                                id: bandEndFrequency
                                Layout.fillWidth: true
                                horizontalAlignment: Text.AlignHCenter
                                text: Common.toLocaleLabel(multibandGatePage.bandFrequencyEnd[bandsListview.currentIndex], 0, "Hz")
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Reaction")
                        }

                        EeSpinBox {
                            label: i18n("Attack")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "AttackTime")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "AttackTime")
                            value: pluginDB[multibandGatePage.bandId + "AttackTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "AttackTime"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Release")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "ReleaseTime")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "ReleaseTime")
                            value: pluginDB[multibandGatePage.bandId + "ReleaseTime"]
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "ReleaseTime"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Gain")
                        }

                        EeSpinBox {
                            label: i18n("Reduction")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "Reduction")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "Reduction")
                            value: pluginDB[multibandGatePage.bandId + "Reduction"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "Reduction"] = v;
                            }
                        }

                        EeSpinBox {
                            id: bandMakeup
                            label: i18n("Makeup")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "Makeup")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "Makeup")
                            value: pluginDB[multibandGatePage.bandId + "Makeup"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "Makeup"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Label {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillWidth: false
                            text: i18n("Curve")
                        }

                        EeSpinBox {
                            label: i18n("Threshold")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "CurveThreshold")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "CurveThreshold")
                            value: pluginDB[multibandGatePage.bandId + "CurveThreshold"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "CurveThreshold"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Zone")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "CurveZone")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "CurveZone")
                            value: pluginDB[multibandGatePage.bandId + "CurveZone"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "CurveZone"] = v;
                            }
                        }
                    }
                }

                Controls.Frame {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.fillWidth: true

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        anchors.fill: parent

                        Controls.Button {
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignCenter
                            text: i18n("Hysteresis")
                            checkable: true
                            checked: pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onCheckedChanged: {
                                if (pluginDB[multibandGatePage.bandId + "Hysteresis"] !== checked) {
                                    pluginDB[multibandGatePage.bandId + "Hysteresis"] = checked;
                                }
                            }
                        }

                        EeSpinBox {
                            label: i18n("Threshold")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "HysteresisThreshold")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "HysteresisThreshold")
                            value: pluginDB[multibandGatePage.bandId + "HysteresisThreshold"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            enabled: pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "HysteresisThreshold"] = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Zone")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue(multibandGatePage.bandId + "HysteresisZone")
                            to: pluginDB.getMaxValue(multibandGatePage.bandId + "HysteresisZone")
                            value: pluginDB[multibandGatePage.bandId + "HysteresisZone"]
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            enabled: pluginDB[multibandGatePage.bandId + "Hysteresis"]
                            onValueModified: v => {
                                pluginDB[multibandGatePage.bandId + "HysteresisZone"] = v;
                            }
                        }
                    }
                }
            }
        }
    }

    Component {
        id: bandSidechainControls

        Column {
            Kirigami.CardsLayout {
                maximumColumns: 8

                anchors {
                    left: parent.left
                    right: parent.right
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Type")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: pluginDB[multibandGatePage.bandId + "SidechainType"]
                    editable: false
                    model: [i18n("Internal"), i18n("External"), i18n("Link")]
                    onActivated: idx => {
                        pluginDB[multibandGatePage.bandId + "SidechainType"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Mode")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: pluginDB[multibandGatePage.bandId + "SidechainMode"]
                    editable: false
                    model: [i18n("Peak"), i18n("RMS"), i18n("LPF"), i18n("SMA")]
                    onActivated: idx => {
                        pluginDB[multibandGatePage.bandId + "SidechainMode"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: pluginDB[multibandGatePage.bandId + "SidechainSource"]
                    editable: false
                    model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]
                    visible: !pluginDB.stereoSplit
                    onActivated: idx => {
                        pluginDB[multibandGatePage.bandId + "SidechainSource"] = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Source")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: pluginDB[multibandGatePage.bandId + "StereoSplitSource"]
                    editable: false
                    model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]
                    visible: pluginDB.stereoSplit
                    onActivated: idx => {
                        pluginDB[multibandGatePage.bandId + "StereoSplitSource"] = idx;
                    }
                }

                EeSpinBox {
                    label: i18n("Preamp")
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: pluginDB.getMinValue(multibandGatePage.bandId + "SidechainPreamp")
                    to: pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainPreamp")
                    value: pluginDB[multibandGatePage.bandId + "SidechainPreamp"]
                    decimals: 2 // Required to show "-inf"
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        pluginDB[multibandGatePage.bandId + "SidechainPreamp"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Reactivity")
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: pluginDB.getMinValue(multibandGatePage.bandId + "SidechainReactivity")
                    to: pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainReactivity")
                    value: pluginDB[multibandGatePage.bandId + "SidechainReactivity"]
                    decimals: 1
                    stepSize: 0.1
                    unit: "ms"
                    onValueModified: v => {
                        pluginDB[multibandGatePage.bandId + "SidechainReactivity"] = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Lookahead")
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: pluginDB.getMinValue(multibandGatePage.bandId + "SidechainLookahead")
                    to: pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainLookahead")
                    value: pluginDB[multibandGatePage.bandId + "SidechainLookahead"]
                    decimals: 1
                    stepSize: 0.1
                    unit: "ms"
                    onValueModified: v => {
                        pluginDB[multibandGatePage.bandId + "SidechainLookahead"] = v;
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Low-Cut")
                        checked: pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"]
                        onCheckedChanged: {
                            pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue(multibandGatePage.bandId + "SidechainLowcutFrequency")
                        to: pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainLowcutFrequency")
                        value: pluginDB[multibandGatePage.bandId + "SidechainLowcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        enabled: pluginDB[multibandGatePage.bandId + "SidechainCustomLowcutFilter"]
                        onValueModified: v => {
                            pluginDB[multibandGatePage.bandId + "SidechainLowcutFrequency"] = v;
                        }
                    }
                }

                ColumnLayout {
                    Controls.CheckBox {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("High-Cut")
                        checked: pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"]
                        onCheckedChanged: {
                            pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"] = checked;
                        }
                    }

                    EeSpinBox {
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue(multibandGatePage.bandId + "SidechainHighcutFrequency")
                        to: pluginDB.getMaxValue(multibandGatePage.bandId + "SidechainHighcutFrequency")
                        value: pluginDB[multibandGatePage.bandId + "SidechainHighcutFrequency"]
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        enabled: pluginDB[multibandGatePage.bandId + "SidechainCustomHighcutFilter"]
                        onValueModified: v => {
                            pluginDB[multibandGatePage.bandId + "SidechainHighcutFrequency"] = v;
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            maximumColumns: 5

            FormCard.FormComboBoxDelegate {
                id: gateMode

                text: i18n("Operating Mode")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: pluginDB.gateMode
                editable: false
                model: [i18n("Classic"), i18n("Modern"), i18n("Linear Phase")]
                onActivated: idx => {
                    pluginDB.gateMode = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: envelopeBoost

                text: i18n("Sidechain Boost")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: pluginDB.envelopeBoost
                editable: false
                model: [i18n("None"), i18n("Pink BT"), i18n("Pink MT"), i18n("Brown BT"), i18n("Brown MT")]
                onActivated: idx => {
                    pluginDB.envelopeBoost = idx;
                }
            }

            FormCard.FormComboBoxDelegate {
                id: comboSideChainInputDevice

                Layout.preferredWidth: gateMode.implicitWidth
                text: i18n("Sidechain Input Device")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                editable: false
                model: PW.ModelNodes
                textRole: "description"
                enabled: pluginDB.externalSidechainEnabled
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

            EeSpinBox {
                id: dry

                label: i18n("Dry")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("dry")
                to: pluginDB.getMaxValue("dry")
                value: pluginDB.dry
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: "dB"
                minusInfinityMode: true
                onValueModified: v => {
                    pluginDB.dry = v;
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
                decimals: 2 // Required to show "-inf"
                stepSize: 0.01
                unit: "dB"
                minusInfinityMode: true
                onValueModified: v => {
                    pluginDB.wet = v;
                }
            }
        }

        RowLayout {
            Kirigami.Card {
                id: bandCard

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignTop

                header: RowLayout {
                    Kirigami.Heading {
                        readonly property string bandTitleTag: multibandGatePage.pluginDB.viewSidechain ? " - " + i18n("Sidechain") : ""
                        text: i18n("Band") + " " + (bandsListview.currentIndex + 1) + bandTitleTag
                        level: 2
                    }

                    Kirigami.ActionToolBar {
                        Layout.margins: Kirigami.Units.smallSpacing
                        alignment: Qt.AlignRight
                        position: Controls.ToolBar.Header
                        flat: true
                        actions: [
                            Kirigami.Action {
                                id: viewLeft
                                checkable: true
                                checked: !multibandGatePage.pluginDB.viewSidechain
                                icon.name: "arrow-left-symbolic"
                                onTriggered: {
                                    if (multibandGatePage.pluginDB.viewSidechain === false) {
                                        return;
                                    }

                                    multibandGatePage.pluginDB.viewSidechain = false;
                                    bandStackview.replace(bandGateControls);
                                }
                            },
                            Kirigami.Action {
                                id: viewRight
                                checkable: true
                                checked: multibandGatePage.pluginDB.viewSidechain
                                icon.name: "arrow-right-symbolic"
                                onTriggered: {
                                    if (multibandGatePage.pluginDB.viewSidechain === true) {
                                        return;
                                    }

                                    multibandGatePage.pluginDB.viewSidechain = true;
                                    bandStackview.replace(bandSidechainControls);
                                }
                            }
                        ]
                    }
                }

                contentItem: Controls.StackView {
                    id: bandStackview

                    implicitWidth: currentItem.implicitWidth
                    implicitHeight: currentItem.implicitHeight
                    initialItem: multibandGatePage.pluginDB.viewSidechain === false ? bandGateControls : bandSidechainControls
                }

                footer: Kirigami.CardsLayout {
                    maximumColumns: 3
                    uniformCellWidths: true

                    anchors {
                        left: parent.left
                        right: parent.right
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns === 3 ? false : true

                        GridLayout {

                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Reduction")
                            }

                            EeAudioLevel {
                                id: reductionLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 72
                                value: Common.toLocaleLabel(multibandGatePage.bandReductionLevelLeft[bandsListview.currentIndex] ?? 0, 0, "")
                                decimals: 0
                                convertDecibelToLinear: true
                                topToBottom: true
                            }

                            EeAudioLevel {
                                id: reductionLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 72
                                value: Common.toLocaleLabel(multibandGatePage.bandReductionLevelRight[bandsListview.currentIndex] ?? 0, 0, "")
                                decimals: 0
                                convertDecibelToLinear: true
                                topToBottom: true
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
                        }
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns == 3 ? false : true

                        GridLayout {
                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Envelope")
                            }

                            EeAudioLevel {
                                id: envelopeLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                value: Common.toLocaleLabel(multibandGatePage.bandEnvelopeLevelLeft[bandsListview.currentIndex] ?? 0, 0, "")
                                decimals: 0
                                convertDecibelToLinear: true
                            }

                            EeAudioLevel {
                                id: envelopeLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                value: Common.toLocaleLabel(multibandGatePage.bandEnvelopeLevelRight[bandsListview.currentIndex] ?? 0, 0, "")
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
                        }
                    }

                    Controls.Frame {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: parent.columns == 3 ? false : true

                        GridLayout {
                            columnSpacing: Kirigami.Units.largeSpacing
                            rowSpacing: Kirigami.Units.largeSpacing
                            columns: 2
                            rows: 3
                            anchors.centerIn: parent

                            Controls.Label {
                                Layout.columnSpan: 2
                                Layout.alignment: Qt.AlignHCenter
                                topPadding: Kirigami.Units.smallSpacing
                                horizontalAlignment: Text.AlignHCenter
                                text: i18n("Curve")
                            }

                            EeAudioLevel {
                                id: curveLevelLeft

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                value: Common.toLocaleLabel(multibandGatePage.bandCurveLevelLeft[bandsListview.currentIndex] ?? 0, 0, "")
                                decimals: 0
                                convertDecibelToLinear: true
                            }

                            EeAudioLevel {
                                id: curveLevelRight

                                Layout.alignment: Qt.AlignBottom
                                implicitWidth: multibandGatePage.radius
                                implicitHeight: multibandGatePage.radius
                                from: Common.minimumDecibelLevel
                                to: 36
                                value: Common.toLocaleLabel(multibandGatePage.bandCurveLevelRight[bandsListview.currentIndex] ?? 0, 0, "")
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
                        }
                    }
                }
            }

            Kirigami.Card {
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.preferredHeight: contentItem.childrenRect.height + 2 * padding
                contentItem: ListView {
                    id: bandsListview

                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredHeight: contentItem.childrenRect.height
                    implicitHeight: contentItem.childrenRect.height

                    model: 8
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    delegate: Controls.ItemDelegate {
                        id: listItemDelegate

                        required property int index

                        hoverEnabled: true
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            ListView.view.currentIndex = index;
                        }

                        contentItem: RowLayout {
                            Controls.Label {
                                Layout.fillWidth: true
                                text: i18n("Band") + " " + (listItemDelegate.index + 1)
                            }

                            Controls.CheckBox {
                                readonly property string bandName: "band" + listItemDelegate.index + "Enable"
                                Layout.alignment: Qt.AlignHCenter
                                visible: listItemDelegate.index > 0
                                checked: listItemDelegate.index > 0 ? pluginDB[bandName] : false
                                onCheckedChanged: {
                                    if (checked != pluginDB[bandName]) {
                                        pluginDB[bandName] = checked;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: multibandGatePage.pluginDB
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
                    checked: pluginBackend ? pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            pluginBackend.showNativeUi();
                        else
                            pluginBackend.closeNativeUi();
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
