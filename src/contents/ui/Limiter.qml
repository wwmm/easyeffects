import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: limiterPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!limiterPage.pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(limiterPage.pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(limiterPage.pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(limiterPage.pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(limiterPage.pluginBackend.getOutputLevelRight());
        gainLevelLeft.setValue(limiterPage.pluginBackend.getGainLevelLeft());
        gainLevelRight.setValue(limiterPage.pluginBackend.getGainLevelRight());
        sideChainLevelLeft.setValue(limiterPage.pluginBackend.getSideChainLevelLeft());
        sideChainLevelRight.setValue(limiterPage.pluginBackend.getSideChainLevelRight());
    }

    Component.onCompleted: {
        limiterPage.pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            maximumColumns: 5
            minimumColumnWidth: Kirigami.Units.gridUnit * 16
            uniformCellWidths: true

            Kirigami.Card {
                id: cardMode

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        Layout.alignment: Qt.AlignTop
                        text: i18n("Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: limiterPage.pluginDB.mode
                        editable: false
                        model: [i18n("Herm thin"), i18n("Herm wide"), i18n("Herm tail"), i18n("Herm duck"), i18n("Exp thin"), i18n("Exp wide"), i18n("Exp tail"), i18n("Exp duck"), i18n("Line thin"), i18n("Line wide"), i18n("Line tail"), i18n("Line duck")] // qmllint disable
                        onActivated: idx => {
                            limiterPage.pluginDB.mode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: oversampling

                        Layout.alignment: Qt.AlignTop
                        text: i18n("Oversampling") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: limiterPage.pluginDB.oversampling
                        editable: false
                        model: [i18n("None"), i18n("Half x2/16 bits"), i18n("Half x2/24 bits"), i18n("Half x3/16 bits"), i18n("Half x3/24 bits"), i18n("Half x4/16 bits"), i18n("Half x4/24 bits"), i18n("Half x6/16 bits"), i18n("Half x6/24 bits"), i18n("Half x8/16 bits"), i18n("Half x8/24 bits"), i18n("Full x2/16 bits"), i18n("Full x2/24 bits"), i18n("Full x3/16 bits"), i18n("Full x3/24 bits"), i18n("Full x4/16 bits"), i18n("Full x4/24 bits"), i18n("Full x6/16 bits"), i18n("Full x6/24 bits"), i18n("Full x8/16 bits"), i18n("Full x8/24 bits"), i18n("True peak/16 bits"), i18n("True peak/24 bits")] // qmllint disable
                        onActivated: idx => {
                            limiterPage.pluginDB.oversampling = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: dithering

                        Layout.alignment: Qt.AlignTop
                        text: i18n("Dithering") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: limiterPage.pluginDB.dithering
                        editable: false
                        model: [i18n("None"), `7 ${Units.bits}`, `8 ${Units.bits}`, `11 ${Units.bits}`, `12 ${Units.bits}`, `15 ${Units.bits}`, `16 ${Units.bits}`, `23 ${Units.bits}`, `24 ${Units.bits}`] // qmllint disable
                        onActivated: idx => {
                            limiterPage.pluginDB.dithering = idx;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLimiter

                header: Kirigami.Heading {
                    text: i18n("Limiter") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSpinBox {
                        id: threshold

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("threshold")
                        to: limiterPage.pluginDB.getMaxValue("threshold")
                        value: limiterPage.pluginDB.threshold
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            limiterPage.pluginDB.threshold = v;
                        }
                    }

                    EeSpinBox {
                        id: attack

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Attack") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("attack")
                        to: limiterPage.pluginDB.getMaxValue("attack")
                        value: limiterPage.pluginDB.attack
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            limiterPage.pluginDB.attack = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Release") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("release")
                        to: limiterPage.pluginDB.getMaxValue("release")
                        value: limiterPage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            limiterPage.pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: stereoLink

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Stereo Link") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("stereoLink")
                        to: limiterPage.pluginDB.getMaxValue("stereoLink")
                        value: limiterPage.pluginDB.stereoLink
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.percent
                        onValueModified: v => {
                            limiterPage.pluginDB.stereoLink = v;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardSideChain

                header: Kirigami.Heading {
                    text: i18n("Sidechain") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    id: cardSideChainColumn

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: sidechainType

                            Layout.columnSpan: 2
                            text: i18n("Type") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: limiterPage.pluginDB.sidechainType
                            editable: false
                            model: [i18n("Internal"), i18n("External"), i18n("Link")] // qmllint disable
                            onActivated: idx => {
                                limiterPage.pluginDB.sidechainType = idx;
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
                            enabled: sidechainType.currentIndex === 1
                            currentIndex: {
                                for (let n = 0; n < PW.ModelNodes.rowCount(); n++) {
                                    if (PW.ModelNodes.getNodeName(n) === limiterPage.pluginDB.sidechainInputDevice)
                                        return n;
                                }
                                return 0;
                            }
                            onActivated: idx => {
                                let selectedName = PW.ModelNodes.getNodeName(idx);
                                if (selectedName !== limiterPage.pluginDB.sidechainInputDevice)
                                    limiterPage.pluginDB.sidechainInputDevice = selectedName;
                            }
                        }

                        EeSpinBox {
                            id: sidechainPreamp

                            label: i18n("Preamp") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("sidechainPreamp")
                            to: limiterPage.pluginDB.getMaxValue("sidechainPreamp")
                            value: limiterPage.pluginDB.sidechainPreamp
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.sidechainPreamp = v;
                            }
                        }

                        EeSpinBox {
                            id: lookahead

                            label: i18n("Lookahead") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("lookahead")
                            to: limiterPage.pluginDB.getMaxValue("lookahead")
                            value: limiterPage.pluginDB.lookahead
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.ms
                            onValueModified: v => {
                                limiterPage.pluginDB.lookahead = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardPreMix

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
                            from: limiterPage.pluginDB.getMinValue("inputToSidechain")
                            to: limiterPage.pluginDB.getMaxValue("inputToSidechain")
                            value: limiterPage.pluginDB.inputToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.inputToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Input to link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("inputToLink")
                            to: limiterPage.pluginDB.getMaxValue("inputToLink")
                            value: limiterPage.pluginDB.inputToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.inputToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("sidechainToInput")
                            to: limiterPage.pluginDB.getMaxValue("sidechainToInput")
                            value: limiterPage.pluginDB.sidechainToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.sidechainToInput = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Sidechain to link") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("sidechainToLink")
                            to: limiterPage.pluginDB.getMaxValue("sidechainToLink")
                            value: limiterPage.pluginDB.sidechainToLink
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.sidechainToLink = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to sidechain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("linkToSidechain")
                            to: limiterPage.pluginDB.getMaxValue("linkToSidechain")
                            value: limiterPage.pluginDB.linkToSidechain
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.linkToSidechain = v;
                            }
                        }

                        EeSpinBox {
                            label: i18n("Link to input") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: limiterPage.pluginDB.getMinValue("linkToInput")
                            to: limiterPage.pluginDB.getMaxValue("linkToInput")
                            value: limiterPage.pluginDB.linkToInput
                            decimals: 2
                            stepSize: 0.01
                            unit: Units.dB
                            minusInfinityMode: true
                            onValueModified: v => {
                                limiterPage.pluginDB.linkToInput = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardALR

                enabled: limiterPage.pluginDB.alr

                header: Kirigami.Heading {
                    text: i18n("Automatic level") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSpinBox {
                        id: alrAttack

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Attack") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("alrAttack")
                        to: limiterPage.pluginDB.getMaxValue("alrAttack")
                        value: limiterPage.pluginDB.alrAttack
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.ms
                        onValueModified: v => {
                            limiterPage.pluginDB.alrAttack = v;
                        }
                    }

                    EeSpinBox {
                        id: alrRelease

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Release") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("alrRelease")
                        to: limiterPage.pluginDB.getMaxValue("alrRelease")
                        value: limiterPage.pluginDB.alrRelease
                        decimals: 1
                        stepSize: 0.1
                        unit: Units.ms
                        onValueModified: v => {
                            limiterPage.pluginDB.alrRelease = v;
                        }
                    }

                    EeSpinBox {
                        id: alrKnee

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Knee") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("alrKnee")
                        to: limiterPage.pluginDB.getMaxValue("alrKnee")
                        value: limiterPage.pluginDB.alrKnee
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            limiterPage.pluginDB.alrKnee = v;
                        }
                    }

                    EeSpinBox {
                        id: alrKneeSmooth

                        Layout.alignment: Qt.AlignTop
                        label: i18n("Smooth") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: limiterPage.pluginDB.getMinValue("alrKneeSmooth")
                        to: limiterPage.pluginDB.getMaxValue("alrKneeSmooth")
                        value: limiterPage.pluginDB.alrKneeSmooth
                        decimals: 2
                        stepSize: 0.01
                        unit: Units.dB
                        onValueModified: v => {
                            limiterPage.pluginDB.alrKneeSmooth = v;
                        }
                    }
                }
            }
        }

        Kirigami.Card {
            id: cardLevels

            Layout.fillWidth: false
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Kirigami.Units.largeSpacing

            contentItem: GridLayout {
                id: levelGridLayout

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
                    text: i18n("Gain") // qmllint disable
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
                    id: gainLevelLeft

                    Layout.alignment: Qt.AlignBottom
                    implicitWidth: levelGridLayout.radius
                    implicitHeight: levelGridLayout.radius
                    from: Common.minimumDecibelLevel
                    to: 0
                    decimals: 0
                    convertDecibelToLinear: true
                    topToBottom: true
                }

                EeAudioLevel {
                    id: gainLevelRight

                    Layout.alignment: Qt.AlignBottom
                    implicitWidth: levelGridLayout.radius
                    implicitHeight: levelGridLayout.radius
                    from: Common.minimumDecibelLevel
                    to: 0
                    decimals: 0
                    convertDecibelToLinear: true
                    topToBottom: true
                }

                EeAudioLevel {
                    id: sideChainLevelLeft

                    Layout.alignment: Qt.AlignBottom
                    Layout.leftMargin: Kirigami.Units.gridUnit
                    implicitWidth: levelGridLayout.radius
                    implicitHeight: levelGridLayout.radius
                    from: Common.minimumDecibelLevel
                    to: 0
                    decimals: 0
                    convertDecibelToLinear: true
                }

                EeAudioLevel {
                    id: sideChainLevelRight

                    Layout.alignment: Qt.AlignBottom
                    implicitWidth: levelGridLayout.radius
                    implicitHeight: levelGridLayout.radius
                    from: Common.minimumDecibelLevel
                    to: 0
                    decimals: 0
                    convertDecibelToLinear: true
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: Units.leftCh
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: Units.rightCh
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.leftMargin: Kirigami.Units.gridUnit
                    horizontalAlignment: Text.AlignHCenter
                    text: Units.leftCh
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: Units.rightCh
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: limiterPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</strong>`) // qmllint disable
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
            Layout.leftMargin: Kirigami.Units.mediumSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 8
            color: Kirigami.Theme.disabledTextColor
        }

        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DbMain.showNativePluginUi
                    checkable: true
                    checked: limiterPage.pluginBackend ? limiterPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            limiterPage.pluginBackend.showNativeUi();
                        else
                            limiterPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Threshold boost") // qmllint disable
                    icon.name: "adjustlevels-symbolic"
                    checkable: true
                    checked: limiterPage.pluginDB.gainBoost
                    onTriggered: {
                        if (limiterPage.pluginDB.gainBoost != checked)
                            limiterPage.pluginDB.gainBoost = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Automatic level") // qmllint disable
                    icon.name: "adjustlevels-symbolic"
                    checkable: true
                    checked: limiterPage.pluginDB.alr
                    onTriggered: {
                        if (limiterPage.pluginDB.alr != checked)
                            limiterPage.pluginDB.alr = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        limiterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
