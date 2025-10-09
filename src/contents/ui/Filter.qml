import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: filterPage

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
        pluginBackend.updateLevelMeters = true;
    }
    Component.onDestruction: {
        if (pluginBackend) {
            pluginBackend.updateLevelMeters = false;
        }
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            implicitWidth: cardLayout.maximumColumnWidth
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: type

                        text: i18n("Type")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.type
                        editable: false
                        model: [i18n("Low-pass"), i18n("High-pass"), i18n("Low-shelf"), i18n("High-shelf"), i18n("Bell"), i18n("Band-pass"), i18n("Notch"), i18n("Resonance"), i18n("Ladder-pass"), i18n("Ladder-rejection"), i18n("All-pass")]
                        onActivated: idx => {
                            pluginDB.type = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        text: i18n("Filter Mode")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.mode
                        editable: false
                        model: ["RLC (BT)", "RLC (MT)", "BWC (BT)", "BWC (MT)", "LRX (BT)", "LRX (MT)", "APO (DR)"]
                        onActivated: idx => {
                            pluginDB.mode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: equalMode

                        text: i18n("Equalizer Mode")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.equalMode
                        editable: false
                        model: ["IIR", "FIR", "FFT", "SPM"]
                        onActivated: idx => {
                            pluginDB.equalMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: slope

                        text: i18n("Slope")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.slope
                        editable: false
                        model: ["x1", "x2", "x3", "x4", "x6", "x8", "x12", "x16"]
                        onActivated: idx => {
                            pluginDB.slope = idx;
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Filter")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: frequency

                        label: i18n("Frequency")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("frequency")
                        to: pluginDB.getMaxValue("frequency")
                        value: pluginDB.frequency
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        onValueModified: v => {
                            pluginDB.frequency = v;
                        }
                    }

                    EeSpinBox {
                        id: width

                        label: i18n("Width")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("width")
                        to: pluginDB.getMaxValue("width")
                        value: pluginDB.width
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            pluginDB.width = v;
                        }
                    }

                    EeSpinBox {
                        id: gain

                        label: i18n("Gain")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("gain")
                        to: pluginDB.getMaxValue("gain")
                        value: pluginDB.gain
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.gain = v;
                        }
                    }

                    EeSpinBox {
                        id: quality

                        label: i18n("Quality")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("quality")
                        to: pluginDB.getMaxValue("quality")
                        value: pluginDB.quality
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            pluginDB.quality = v;
                        }
                    }

                    EeSpinBox {
                        id: balance

                        label: i18n("Balance")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
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
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: filterPage.pluginDB
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
