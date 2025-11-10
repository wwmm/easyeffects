import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigami.layouts as KirigamiLayouts
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: autogainPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        momentary.value = pluginBackend.getMomentaryLevel();
        shortterm.value = pluginBackend.getShorttermLevel();
        integrated.value = pluginBackend.getIntegratedLevel();
        relative.value = pluginBackend.getRelativeLevel();
        range.value = pluginBackend.getRangeLevel();
        loudness.value = pluginBackend.getLoudnessLevel();
        outputGain.value = pluginBackend.getOutputGainLevel();
        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            uniformCellWidths: true

            EeCard {
                id: cardControls

                implicitWidth: cardLayout.maximumColumnWidth

                title: i18n("Controls")// qmllint disable

                FormCard.FormComboBoxDelegate {
                    id: reference

                    text: i18n("Reference")// qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    verticalPadding: 0
                    currentIndex: autogainPage.pluginDB.reference
                    editable: false
                    model: [i18n("Momentary"), i18n("Short-term"), i18n("Integrated"), i18n("Geometric mean (MSI)"), i18n("Geometric mean (MS)"), i18n("Geometric mean (MI)"), i18n("Geometric mean (SI)")]// qmllint disable
                    onActivated: idx => {
                        autogainPage.pluginDB.reference = idx;
                    }
                }

                EeSpinBox {
                    id: target

                    label: i18n("Target")// qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("target")
                    to: autogainPage.pluginDB.getMaxValue("target")
                    value: autogainPage.pluginDB.target
                    decimals: 2
                    stepSize: 0.1
                    unit: "dB"
                    onValueModified: v => {
                        autogainPage.pluginDB.target = v;
                    }
                }

                EeSpinBox {
                    id: silenceThreshold

                    label: i18n("Silence")// qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("silenceThreshold")
                    to: autogainPage.pluginDB.getMaxValue("silenceThreshold")
                    value: autogainPage.pluginDB.silenceThreshold
                    decimals: 2
                    stepSize: 0.1
                    unit: "dB"
                    onValueModified: v => {
                        autogainPage.pluginDB.silenceThreshold = v;
                    }
                }

                EeSpinBox {
                    id: maximumHistory

                    label: i18n("Maximum history")// qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: autogainPage.pluginDB.getMinValue("maximumHistory")
                    to: autogainPage.pluginDB.getMaxValue("maximumHistory")
                    value: autogainPage.pluginDB.maximumHistory
                    decimals: 0
                    stepSize: 1
                    unit: "s"
                    onValueModified: v => {
                        autogainPage.pluginDB.maximumHistory = v;
                    }
                }

                Item {
                    Layout.fillHeight: true
                }
            }

            EeCard {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                title: i18n("Loudness")// qmllint disable

                EeProgressBar {
                    id: momentary

                    label: i18n("Momentary")// qmllint disable
                    unit: "LUFS"
                    from: Common.minimumDecibelLevel
                    to: 10
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: shortterm

                    label: i18n("Short-term")// qmllint disable
                    unit: "LUFS"
                    from: Common.minimumDecibelLevel
                    to: 10
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: integrated

                    label: i18n("Integrated")// qmllint disable
                    unit: "LUFS"
                    from: Common.minimumDecibelLevel
                    to: 10
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: relative

                    label: i18n("Relative")// qmllint disable
                    unit: "LUFS"
                    from: Common.minimumDecibelLevel
                    to: 10
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: range

                    label: i18n("Range")// qmllint disable
                    unit: "LU"
                    from: 0
                    to: 50
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: loudness

                    label: i18n("Loudness")// qmllint disable
                    unit: "LUFS"
                    from: Common.minimumDecibelLevel
                    to: 10
                    value: 0
                    decimals: 1
                }

                EeProgressBar {
                    id: outputGain

                    label: i18n("Output gain")// qmllint disable
                    unit: "dB"
                    from: Common.minimumDecibelLevel
                    to: 20
                    value: 0
                    decimals: 1
                    convertDecibelToLinear: true
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: autogainPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.ebur128}</strong>`)// qmllint disable
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
                    text: i18n("Reset history")// qmllint disable
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Force silence")// qmllint disable
                    tooltip: i18n("Force silence when the level is below the silence threshold")// qmllint disable
                    icon.name: "audio-volume-muted-symbolic"
                    checkable: true
                    checked: autogainPage.pluginDB.forceSilence
                    onTriggered: {
                        if (checked !== autogainPage.pluginDB.forceSilence) {
                            autogainPage.pluginDB.forceSilence = checked;
                        }
                    }
                },
                Kirigami.Action {
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Reset settings")// qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
