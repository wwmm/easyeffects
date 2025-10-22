import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: expanderPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = expanderPage.pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = expanderPage.pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = expanderPage.pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = expanderPage.pluginBackend.getOutputLevelRight();
        reductionLevelLeft.value = expanderPage.pluginBackend.getReductionLevelLeft();
        reductionLevelRight.value = expanderPage.pluginBackend.getReductionLevelRight();
        sideChainLevelLeft.value = expanderPage.pluginBackend.getSideChainLevelLeft();
        sideChainLevelRight.value = expanderPage.pluginBackend.getSideChainLevelRight();
        curveLevelLeft.value = expanderPage.pluginBackend.getCurveLevelLeft();
        curveLevelRight.value = expanderPage.pluginBackend.getCurveLevelRight();
        envelopeLevelLeft.value = expanderPage.pluginBackend.getEnvelopeLevelLeft();
        envelopeLevelRight.value = expanderPage.pluginBackend.getEnvelopeLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            maximumColumns: 5
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Expander") // qmllint disable
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
                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.mode
                            editable: false
                            model: [i18n("Downward"), i18n("Upward")]// qmllint disable
                            onActivated: idx => {
                                expanderPage.pluginDB.mode = idx;
                            }
                        }

                        EeSpinBox {
                            id: ratio

                            label: i18n("Ratio") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("ratio")
                            to: expanderPage.pluginDB.getMaxValue("ratio")
                            value: expanderPage.pluginDB.ratio
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                expanderPage.pluginDB.ratio = v;
                            }
                        }

                        EeSpinBox {
                            id: knee

                            label: i18n("Knee") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("knee")
                            to: expanderPage.pluginDB.getMaxValue("knee")
                            value: expanderPage.pluginDB.knee
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                expanderPage.pluginDB.knee = v;
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

                            label: i18n("Attack") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("threshold")
                            to: expanderPage.pluginDB.getMaxValue("threshold")
                            value: expanderPage.pluginDB.threshold
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            onValueModified: v => {
                                expanderPage.pluginDB.threshold = v;
                            }
                        }

                        EeSpinBox {
                            id: attack

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("attack")
                            to: expanderPage.pluginDB.getMaxValue("attack")
                            value: expanderPage.pluginDB.attack
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                expanderPage.pluginDB.attack = v;
                            }
                        }

                        EeSpinBox {
                            id: releaseThreshold

                            label: i18n("Release") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("releaseThreshold")
                            to: expanderPage.pluginDB.getMaxValue("releaseThreshold")
                            value: expanderPage.pluginDB.releaseThreshold
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                expanderPage.pluginDB.releaseThreshold = v;
                            }
                        }

                        EeSpinBox {
                            id: release

                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("release")
                            to: expanderPage.pluginDB.getMaxValue("release")
                            value: expanderPage.pluginDB.release
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                expanderPage.pluginDB.release = v;
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
                            text: i18n("Type") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.sidechainType
                            editable: false
                            model: [i18n("Internal"), i18n("External"), i18n("Link")]// qmllint disable
                            onActivated: idx => {
                                expanderPage.pluginDB.sidechainType = idx;
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
                                    if (PW.ModelNodes.getNodeName(n) === expanderPage.pluginDB.sidechainInputDevice)
                                        return n;
                                }
                                return 0;
                            }
                            onActivated: idx => {
                                let selectedName = PW.ModelNodes.getNodeName(idx);
                                if (selectedName !== expanderPage.pluginDB.sidechainInputDevice)
                                    expanderPage.pluginDB.sidechainInputDevice = selectedName;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainMode

                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.sidechainMode
                            editable: false
                            model: [i18n("Peak"), i18n("RMS"), i18n("Low-Pass"), i18n("SMA")]// qmllint disable
                            onActivated: idx => {
                                expanderPage.pluginDB.sidechainMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: sidechainSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.sidechainSource
                            editable: false
                            model: [i18n("Middle"), i18n("Side"), i18n("Left"), i18n("Right"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: !expanderPage.pluginDB.stereoSplit
                            onActivated: idx => {
                                expanderPage.pluginDB.sidechainSource = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: stereoSplitSource

                            text: i18n("Source") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.stereoSplitSource
                            editable: false
                            model: [i18n("Left/Right"), i18n("Right/Left"), i18n("Mid/Side"), i18n("Side/Mid"), i18n("Min"), i18n("Max")]// qmllint disable
                            visible: expanderPage.pluginDB.stereoSplit
                            onActivated: idx => {
                                expanderPage.pluginDB.stereoSplitSource = idx;
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

                            label: i18n("Preamp") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("sidechainPreamp")
                            to: expanderPage.pluginDB.getMaxValue("sidechainPreamp")
                            value: expanderPage.pluginDB.sidechainPreamp
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                expanderPage.pluginDB.sidechainPreamp = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainReactivity

                            label: i18n("Reactivity") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("sidechainReactivity")
                            to: expanderPage.pluginDB.getMaxValue("sidechainReactivity")
                            value: expanderPage.pluginDB.sidechainReactivity
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                expanderPage.pluginDB.sidechainReactivity = v;
                            }
                        }

                        EeSpinBox {
                            id: sidechainLookahead

                            Layout.columnSpan: 2
                            label: i18n("Lookahead") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("sidechainLookahead")
                            to: expanderPage.pluginDB.getMaxValue("sidechainLookahead")
                            value: expanderPage.pluginDB.sidechainLookahead
                            decimals: 3
                            stepSize: 0.001
                            unit: "ms"
                            onValueModified: v => {
                                expanderPage.pluginDB.sidechainLookahead = v;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: hpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("High-Pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.hpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]// qmllint disable
                            onActivated: idx => {
                                expanderPage.pluginDB.hpfMode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: lpfMode

                            horizontalPadding: 0
                            verticalPadding: 0
                            text: i18n("Low-Pass") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: expanderPage.pluginDB.lpfMode
                            editable: false
                            model: [i18n("Off"), i18n("12 dB/oct"), i18n("24 dB/oct"), i18n("36 dB/oct")]// qmllint disable
                            onActivated: idx => {
                                expanderPage.pluginDB.lpfMode = idx;
                            }
                        }

                        EeSpinBox {
                            id: hpfFrequency

                            horizontalPadding: 0
                            verticalPadding: 0
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: expanderPage.pluginDB.getMinValue("hpfFrequency")
                            to: expanderPage.pluginDB.getMaxValue("hpfFrequency")
                            value: expanderPage.pluginDB.hpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: hpfMode.currentIndex !== 0
                            onValueModified: v => {
                                expanderPage.pluginDB.hpfFrequency = v;
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
                            from: expanderPage.pluginDB.getMinValue("lpfFrequency")
                            to: expanderPage.pluginDB.getMaxValue("lpfFrequency")
                            value: expanderPage.pluginDB.lpfFrequency
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            visible: lpfMode.currentIndex !== 0
                            onValueModified: v => {
                                expanderPage.pluginDB.lpfFrequency = v;
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

                contentItem: Column {
                    EeSpinBox {
                        id: dry

                        label: i18n("Dry") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("dry")
                        to: expanderPage.pluginDB.getMaxValue("dry")
                        value: expanderPage.pluginDB.dry
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.dry = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("wet")
                        to: expanderPage.pluginDB.getMaxValue("wet")
                        value: expanderPage.pluginDB.wet
                        decimals: 2 // Required to show "-inf"
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: v => {
                            expanderPage.pluginDB.wet = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: makeup

                        Layout.columnSpan: 2
                        label: i18n("Makeup") // qmllint disable
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: expanderPage.pluginDB.getMinValue("makeup")
                        to: expanderPage.pluginDB.getMaxValue("makeup")
                        value: expanderPage.pluginDB.makeup
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            expanderPage.pluginDB.makeup = v;
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
                        to: 60
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
                        to: 60
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

        pluginDB: expanderPage.pluginDB
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
                    text: i18n("Show Native Window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: expanderPage.pluginBackend ? expanderPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            expanderPage.pluginBackend.showNativeUi();
                        else
                            expanderPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: expanderPage.pluginDB.sidechainListen
                    onTriggered: {
                        if (expanderPage.pluginDB.sidechainListen != checked)
                            expanderPage.pluginDB.sidechainListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Stereo Split") // qmllint disable
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: expanderPage.pluginDB.stereoSplit
                    onTriggered: {
                        if (expanderPage.pluginDB.stereoSplit != checked)
                            expanderPage.pluginDB.stereoSplit = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        expanderPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
