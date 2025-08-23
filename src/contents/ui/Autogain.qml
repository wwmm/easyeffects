import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: autogainPage

    required property var name
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

            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: Column {
                    FormCard.FormComboBoxDelegate {
                        id: reference

                        text: i18n("Reference")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: autogainPage.pluginDB.reference
                        editable: false
                        model: [i18n("Momentary"), i18n("Short-Term"), i18n("Integrated"), i18n("Geometric Mean (MSI)"), i18n("Geometric Mean (MS)"), i18n("Geometric Mean (MI)"), i18n("Geometric Mean (SI)")]
                        onActivated: idx => {
                            autogainPage.pluginDB.reference = idx;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: target

                        label: i18n("Target")
                        from: autogainPage.pluginDB.getMinValue("target")
                        to: autogainPage.pluginDB.getMaxValue("target")
                        value: autogainPage.pluginDB.target
                        decimals: 2
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            autogainPage.pluginDB.target = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: silenceThreshold

                        label: i18n("Silence")
                        from: autogainPage.pluginDB.getMinValue("silenceThreshold")
                        to: autogainPage.pluginDB.getMaxValue("silenceThreshold")
                        value: autogainPage.pluginDB.silenceThreshold
                        decimals: 2
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            autogainPage.pluginDB.silenceThreshold = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeSpinBox {
                        id: maximumHistory

                        label: i18n("Maximum History")
                        from: autogainPage.pluginDB.getMinValue("maximumHistory")
                        to: autogainPage.pluginDB.getMaxValue("maximumHistory")
                        value: autogainPage.pluginDB.maximumHistory
                        decimals: 0
                        stepSize: 1
                        unit: "s"
                        onValueModified: v => {
                            autogainPage.pluginDB.maximumHistory = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Loudness")
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeProgressBar {
                        id: momentary

                        label: i18n("Momentary")
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: shortterm

                        label: i18n("Short-Term")
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: integrated

                        label: i18n("Integrated")
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: relative

                        label: i18n("Relative")
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: range

                        label: i18n("Range")
                        unit: "LU"
                        from: 0
                        to: 50
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: loudness

                        label: i18n("Loudness")
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: outputGain

                        label: i18n("Output Gain")
                        unit: "dB"
                        from: Common.minimumDecibelLevel
                        to: 20
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.ebur128}</b>`)
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
                    text: i18n("Reset History")
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Force Silence")
                    tooltip: i18n("Force Silence When the Level is Below the Silence Threshold")
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
                    text: i18n("Reset Settings")
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        autogainPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
