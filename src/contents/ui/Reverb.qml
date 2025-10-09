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
                        id: roomSize

                        Layout.alignment: Qt.AlignTop
                        text: i18n("Room Size")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.roomSize
                        editable: false
                        model: [i18n("Small"), i18n("Medium"), i18n("Large"), i18n("Tunnel-like"), i18n("Large/smooth"), i18n("Experimental")]
                        onActivated: idx => {
                            pluginDB.roomSize = idx;
                        }
                    }

                    EeSpinBox {
                        id: decayTime

                        label: i18n("Decay Time")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pluginDB.getMinValue("decayTime")
                        to: pluginDB.getMaxValue("decayTime")
                        value: pluginDB.decayTime
                        decimals: 2
                        stepSize: 0.01
                        unit: "s"
                        onValueModified: v => {
                            pluginDB.decayTime = v;
                        }
                    }

                    EeSpinBox {
                        id: predelay

                        label: i18n("Pre Delay")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pluginDB.getMinValue("predelay")
                        to: pluginDB.getMaxValue("predelay")
                        value: pluginDB.predelay
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            pluginDB.predelay = v;
                        }
                    }

                    EeSpinBox {
                        id: diffusion

                        label: i18n("Diffusion")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pluginDB.getMinValue("diffusion")
                        to: pluginDB.getMaxValue("diffusion")
                        value: pluginDB.diffusion
                        decimals: 2
                        stepSize: 0.01
                        unit: "%"
                        onValueModified: v => {
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
                            onValueModified: v => {
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
                            onValueModified: v => {
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
                            onValueModified: v => {
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
                            onValueModified: v => {
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
                            onValueModified: v => {
                                pluginDB.amount = v;
                            }
                        }
                    }
                }
            }
        }
    }

    Kirigami.MenuDialog {
        id: presetsDialog

        title: i18n("Reverberation Presets")
        actions: [
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Ambience")
                onTriggered: {
                    pluginDB.decayTime = 1.10354;
                    pluginDB.hfDamp = 2182.58;
                    pluginDB.roomSize = 4;
                    pluginDB.diffusion = 0.69;
                    pluginDB.amount = Common.linearTodb(0.291183);
                    pluginDB.dry = Common.linearTodb(1);
                    pluginDB.predelay = 6.5;
                    pluginDB.bassCut = 514.079;
                    pluginDB.trebleCut = 4064.15;
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Empty Walls")
                onTriggered: {
                    pluginDB.decayTime = 0.505687;
                    pluginDB.hfDamp = 3971.64;
                    pluginDB.roomSize = 4;
                    pluginDB.diffusion = 0.17;
                    pluginDB.amount = Common.linearTodb(0.198884);
                    pluginDB.dry = Common.linearTodb(1);
                    pluginDB.predelay = 13;
                    pluginDB.bassCut = 240.453;
                    pluginDB.trebleCut = 3303.47;
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Room")
                onTriggered: {
                    pluginDB.decayTime = 0.445945;
                    pluginDB.hfDamp = 5508.46;
                    pluginDB.roomSize = 4;
                    pluginDB.diffusion = 0.54;
                    pluginDB.amount = Common.linearTodb(0.469761);
                    pluginDB.dry = Common.linearTodb(1);
                    pluginDB.predelay = 25;
                    pluginDB.bassCut = 257.65;
                    pluginDB.trebleCut = 20000;
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Large Empty Hall")
                onTriggered: {
                    pluginBackend.reset();
                    pluginDB.decayTime = 2.00689;
                    pluginDB.hfDamp = 20000;
                    pluginDB.amount = Common.linearTodb(0.366022);
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Disco")
                onTriggered: {
                    pluginBackend.reset();
                    pluginDB.decayTime = 1;
                    pluginDB.hfDamp = 3396.49;
                    pluginDB.amount = Common.linearTodb(0.269807);
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Large Occupied Hall")
                onTriggered: {
                    pluginBackend.reset();
                    pluginDB.decayTime = 1;
                    pluginDB.hfDamp = 3396.49;
                    pluginDB.amount = Common.linearTodb(0.269807);
                }
            }
        ]
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
                    text: i18n("Presets")
                    icon.name: "bookmarks-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    onTriggered: {
                        presetsDialog.open();
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
