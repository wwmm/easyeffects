import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: reverbPage

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
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: roomSize

                        Layout.alignment: Qt.AlignTop
                        text: i18n("Room Size")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.roomSize
                        editable: false
                        model: [i18n("Small"), i18n("Medium"), i18n("Large"), i18n("Tunnel-like"), i18n("Large/smooth"), i18n("Experimental")]
                        onActivated: (idx) => {
                            pluginDB.roomSize = idx;
                        }
                    }

                    EeSpinBox {
                        id: decayTime

                        label: i18n("Decay Time")
                        from: pluginDB.getMinValue("decayTime")
                        to: pluginDB.getMaxValue("decayTime")
                        value: pluginDB.decayTime
                        decimals: 2
                        stepSize: 0.01
                        unit: "s"
                        onValueModified: (v) => {
                            pluginDB.decayTime = v;
                        }
                    }

                    EeSpinBox {
                        id: predelay

                        label: i18n("Pre Delay")
                        from: pluginDB.getMinValue("predelay")
                        to: pluginDB.getMaxValue("predelay")
                        value: pluginDB.predelay
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.predelay = v;
                        }
                    }

                    EeSpinBox {
                        id: diffusion

                        label: i18n("Diffusion")
                        from: pluginDB.getMinValue("diffusion")
                        to: pluginDB.getMaxValue("diffusion")
                        value: pluginDB.diffusion
                        decimals: 2
                        stepSize: 0.01
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.diffusion = v;
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
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: hfDamp

                            Layout.columnSpan: 2
                            label: i18n("High Frequency Damping")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("hfDamp")
                            to: pluginDB.getMaxValue("hfDamp")
                            value: pluginDB.hfDamp
                            decimals: 2
                            stepSize: 0.01
                            unit: "Hz"
                            onValueModified: (v) => {
                                pluginDB.hfDamp = v;
                            }
                        }

                        EeSpinBox {
                            id: bassCut

                            label: i18n("Bass Cut")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("bassCut")
                            to: pluginDB.getMaxValue("bassCut")
                            value: pluginDB.bassCut
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            onValueModified: (v) => {
                                pluginDB.bassCut = v;
                            }
                        }

                        EeSpinBox {
                            id: trebleCut

                            label: i18n("Treble Cut")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("trebleCut")
                            to: pluginDB.getMaxValue("trebleCut")
                            value: pluginDB.trebleCut
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            onValueModified: (v) => {
                                pluginDB.trebleCut = v;
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
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
                                pluginDB.dry = v;
                            }
                        }

                        EeSpinBox {
                            id: wet

                            label: i18n("Wet")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("amount")
                            to: pluginDB.getMaxValue("amount")
                            value: pluginDB.amount
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
                                pluginDB.amount = v;
                            }
                        }

                    }

                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: reverbPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.calf}</b>`)
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
