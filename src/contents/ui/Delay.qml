import "Common.js" as Common
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
            return ;

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

                        FormCard.FormComboBoxDelegate {
                            id: modeL

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.modeL
                            editable: false
                            model: [i18n("Samples"), i18n("Distance"), i18n("Time")]
                            onActivated: (idx) => {
                                pluginDB.modeL = idx;
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
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
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
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
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

                        FormCard.FormComboBoxDelegate {
                            id: modeR

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.modeR
                            editable: false
                            model: [i18n("Samples"), i18n("Distance"), i18n("Time")]
                            onActivated: (idx) => {
                                pluginDB.modeR = idx;
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
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
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
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
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
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.show_native_ui();
                        else
                            pluginBackend.close_native_ui();
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
