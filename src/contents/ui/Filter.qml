import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: filterPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = filterPage.pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = filterPage.pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = filterPage.pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = filterPage.pluginBackend.getOutputLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            implicitWidth: cardLayout.maximumColumnWidth
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: type

                        text: i18n("Type") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: filterPage.pluginDB.type
                        editable: false
                        model: [i18n("Low-pass"), i18n("High-pass"), i18n("Low-shelf"), i18n("High-shelf"), i18n("Bell"), i18n("Band-pass"), i18n("Notch"), i18n("Resonance"), i18n("Ladder-pass"), i18n("Ladder-rejection"), i18n("All-pass")]// qmllint disable
                        onActivated: idx => {
                            filterPage.pluginDB.type = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: mode

                        text: i18n("Filter Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: filterPage.pluginDB.mode
                        editable: false
                        model: ["RLC (BT)", "RLC (MT)", "BWC (BT)", "BWC (MT)", "LRX (BT)", "LRX (MT)", "APO (DR)"]
                        onActivated: idx => {
                            filterPage.pluginDB.mode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: equalMode

                        text: i18n("Equalizer Mode") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: filterPage.pluginDB.equalMode
                        editable: false
                        model: ["IIR", "FIR", "FFT", "SPM"]
                        onActivated: idx => {
                            filterPage.pluginDB.equalMode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: slope

                        text: i18n("Slope") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: filterPage.pluginDB.slope
                        editable: false
                        model: ["x1", "x2", "x3", "x4", "x6", "x8", "x12", "x16"]
                        onActivated: idx => {
                            filterPage.pluginDB.slope = idx;
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Filter") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: frequency

                        label: i18n("Frequency") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: filterPage.pluginDB.getMinValue("frequency")
                        to: filterPage.pluginDB.getMaxValue("frequency")
                        value: filterPage.pluginDB.frequency
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        onValueModified: v => {
                            filterPage.pluginDB.frequency = v;
                        }
                    }

                    EeSpinBox {
                        id: width

                        label: i18n("Width") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: filterPage.pluginDB.getMinValue("width")
                        to: filterPage.pluginDB.getMaxValue("width")
                        value: filterPage.pluginDB.width
                        decimals: 0
                        stepSize: 1
                        onValueModified: v => {
                            filterPage.pluginDB.width = v;
                        }
                    }

                    EeSpinBox {
                        id: gain

                        label: i18n("Gain") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: filterPage.pluginDB.getMinValue("gain")
                        to: filterPage.pluginDB.getMaxValue("gain")
                        value: filterPage.pluginDB.gain
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            filterPage.pluginDB.gain = v;
                        }
                    }

                    EeSpinBox {
                        id: quality

                        label: i18n("Quality") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: filterPage.pluginDB.getMinValue("quality")
                        to: filterPage.pluginDB.getMaxValue("quality")
                        value: filterPage.pluginDB.quality
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: v => {
                            filterPage.pluginDB.quality = v;
                        }
                    }

                    EeSpinBox {
                        id: balance

                        label: i18n("Balance") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: filterPage.pluginDB.getMinValue("balance")
                        to: filterPage.pluginDB.getMaxValue("balance")
                        value: filterPage.pluginDB.balance
                        decimals: 1
                        stepSize: 0.1
                        unit: "%"
                        onValueModified: v => {
                            filterPage.pluginDB.balance = v;
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
                    checked: filterPage.pluginBackend ? filterPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            filterPage.pluginBackend.showNativeUi();
                        else
                            filterPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        filterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
