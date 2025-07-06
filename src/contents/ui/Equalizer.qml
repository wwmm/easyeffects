import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: equalizerPage

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
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {

        Kirigami.CardsLayout {
            maximumColumns: 5
            readonly property real columnSize: pitchLeft.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize

            FormCard.FormComboBoxDelegate {
                id: mode

                text: i18n("Mode")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: pluginDB.mode
                editable: false
                model: [i18n("IIR"), i18n("FIR"), i18n("FFT"), i18n("SPM")]
                onActivated: idx => {
                    pluginDB.mode = idx;
                }
            }

            EeSpinBox {
                id: numBands

                label: i18n("Bands")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("numBands")
                to: pluginDB.getMaxValue("numBands")
                value: pluginDB.numBands
                decimals: 0
                stepSize: 1
                onValueModified: v => {
                    pluginDB.numBands = v;
                }
            }

            EeSpinBox {
                id: balance

                label: i18n("Balance")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("balance")
                to: pluginDB.getMaxValue("balance")
                value: pluginDB.balance
                decimals: 1
                stepSize: 0.1
                unit: "%"
                onValueModified: v => {
                    pluginDB.balance = v;
                }
            }

            EeSpinBox {
                id: pitchLeft

                label: i18n("Pitch Left")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("pitchLeft")
                to: pluginDB.getMaxValue("pitchLeft")
                value: pluginDB.pitchLeft
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    pluginDB.pitchLeft = v;
                }
            }

            EeSpinBox {
                id: pitchRight

                label: i18n("Pitch Right")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("pitchRight")
                to: pluginDB.getMaxValue("pitchRight")
                value: pluginDB.pitchRight
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    pluginDB.pitchRight = v;
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: equalizerPage.pluginDB
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
                    text: i18n("Split Channels")
                    icon.name: "split-symbolic"
                    checkable: true
                    checked: pluginDB.splitChannels
                    onTriggered: {
                        if (pluginDB.splitChannels != checked)
                            pluginDB.splitChannels = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Flat Response")
                    icon.name: "map-flat-symbolic"
                    onTriggered: {
                        pluginBackend.flatResponse();
                    }
                },
                Kirigami.Action {
                    text: i18n("Calculate Frequencies")
                    icon.name: "folder-calculate-symbolic"
                    onTriggered: {
                        pluginBackend.calculateFrequencies();
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
