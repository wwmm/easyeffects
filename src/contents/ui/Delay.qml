import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: delayPage

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
            id: cardLayout

            implicitWidth: cardLayout.maximumColumnWidth
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Left")
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: modeL

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.modeL
                            editable: false
                            model: [i18n("Samples"), i18n("Distance"), i18n("Time")]
                            onActivated: idx => {
                                pluginDB.modeL = idx;
                            }
                        }

                        EeSpinBox {
                            id: sampleL

                            Layout.columnSpan: 2
                            visible: pluginDB.modeL === 0 ? true : false
                            label: i18n("Samples")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sampleL")
                            to: pluginDB.getMaxValue("sampleL")
                            value: pluginDB.sampleL
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pluginDB.sampleL = v;
                            }
                        }

                        EeSpinBox {
                            id: timeL

                            Layout.columnSpan: 2
                            visible: pluginDB.modeL === 2 ? true : false
                            label: i18n("Time")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("timeL")
                            to: pluginDB.getMaxValue("timeL")
                            value: pluginDB.timeL
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.timeL = v;
                            }
                        }

                        EeSpinBox {
                            id: metersL

                            visible: pluginDB.modeL === 1 ? true : false
                            label: i18n("Meters")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("metersL")
                            to: pluginDB.getMaxValue("metersL")
                            value: pluginDB.metersL
                            decimals: 0
                            stepSize: 1
                            unit: "m"
                            onValueModified: v => {
                                pluginDB.metersL = v;
                            }
                        }

                        EeSpinBox {
                            id: centimetersL

                            visible: pluginDB.modeL === 1 ? true : false
                            label: i18n("Centimeters")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("centimetersL")
                            to: pluginDB.getMaxValue("centimetersL")
                            value: pluginDB.centimetersL
                            decimals: 1
                            stepSize: 0.1
                            unit: "cm"
                            onValueModified: v => {
                                pluginDB.centimetersL = v;
                            }
                        }

                        EeSpinBox {
                            id: temperatureL

                            Layout.columnSpan: 2
                            visible: pluginDB.modeL === 1 ? true : false
                            label: i18n("Temperature")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("temperatureL")
                            to: pluginDB.getMaxValue("temperatureL")
                            value: pluginDB.temperatureL
                            decimals: 1
                            stepSize: 0.1
                            unit: "°C"
                            onValueModified: v => {
                                pluginDB.temperatureL = v;
                            }
                        }

                        EeSpinBox {
                            id: dryL

                            label: i18n("Dry")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("dryL")
                            to: pluginDB.getMaxValue("dryL")
                            value: pluginDB.dryL
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.dryL = v;
                            }
                        }

                        EeSpinBox {
                            id: wetL

                            label: i18n("Wet")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("wetL")
                            to: pluginDB.getMaxValue("wetL")
                            value: pluginDB.wetL
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.wetL = v;
                            }
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Right")
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: modeR

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.modeR
                            editable: false
                            model: [i18n("Samples"), i18n("Distance"), i18n("Time")]
                            onActivated: idx => {
                                pluginDB.modeR = idx;
                            }
                        }

                        EeSpinBox {
                            id: sampleR

                            Layout.columnSpan: 2
                            visible: pluginDB.modeR === 0 ? true : false
                            label: i18n("Samples")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sampleR")
                            to: pluginDB.getMaxValue("sampleR")
                            value: pluginDB.sampleR
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pluginDB.sampleR = v;
                            }
                        }

                        EeSpinBox {
                            id: timeR

                            Layout.columnSpan: 2
                            visible: pluginDB.modeR === 2 ? true : false
                            label: i18n("Time")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("timeR")
                            to: pluginDB.getMaxValue("timeR")
                            value: pluginDB.timeR
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: v => {
                                pluginDB.timeR = v;
                            }
                        }

                        EeSpinBox {
                            id: metersR

                            visible: pluginDB.modeR === 1 ? true : false
                            label: i18n("Meters")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("metersR")
                            to: pluginDB.getMaxValue("metersR")
                            value: pluginDB.metersR
                            decimals: 0
                            stepSize: 1
                            unit: "m"
                            onValueModified: v => {
                                pluginDB.metersR = v;
                            }
                        }

                        EeSpinBox {
                            id: centimetersR

                            visible: pluginDB.modeR === 1 ? true : false
                            label: i18n("Centimeters")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("centimetersR")
                            to: pluginDB.getMaxValue("centimetersR")
                            value: pluginDB.centimetersR
                            decimals: 1
                            stepSize: 0.1
                            unit: "cm"
                            onValueModified: v => {
                                pluginDB.centimetersR = v;
                            }
                        }

                        EeSpinBox {
                            id: temperatureR

                            Layout.columnSpan: 2
                            visible: pluginDB.modeR === 1 ? true : false
                            label: i18n("Temperature")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("temperatureR")
                            to: pluginDB.getMaxValue("temperatureR")
                            value: pluginDB.temperatureR
                            decimals: 1
                            stepSize: 0.1
                            unit: "°C"
                            onValueModified: v => {
                                pluginDB.temperatureR = v;
                            }
                        }

                        EeSpinBox {
                            id: dryR

                            label: i18n("Dry")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("dryR")
                            to: pluginDB.getMaxValue("dryR")
                            value: pluginDB.dryR
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.dryR = v;
                            }
                        }

                        EeSpinBox {
                            id: wetR

                            label: i18n("Wet")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("wetR")
                            to: pluginDB.getMaxValue("wetR")
                            value: pluginDB.wetR
                            decimals: 2 // Required to show "-inf"
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: v => {
                                pluginDB.wetR = v;
                            }
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: delayPage.pluginDB
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
                    text: i18n("Invert Left")
                    icon.name: "edit-select-invert-symbolic"
                    checkable: true
                    checked: pluginDB.invertPhaseL
                    onTriggered: {
                        if (checked !== pluginDB.invertPhaseL)
                            pluginDB.invertPhaseL = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Invert Right")
                    icon.name: "edit-select-invert-symbolic"
                    checkable: true
                    checked: pluginDB.invertPhaseR
                    onTriggered: {
                        if (checked !== pluginDB.invertPhaseR)
                            pluginDB.invertPhaseR = checked;
                    }
                },
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
