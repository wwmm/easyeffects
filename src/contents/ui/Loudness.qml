import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: loudnessPage

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

            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: std

                            text: i18n("Contour")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.std
                            editable: false
                            model: ["Flat", "ISO226-2003", "Fletcher-Munson", "Robinson-Dadson"]
                            onActivated: idx => {
                                pluginDB.std = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: fft

                            text: i18n("FFT Size")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.fft
                            editable: false
                            model: [256, 512, 1024, 2048, 4096, 8192, 16384]
                            onActivated: idx => {
                                pluginDB.fft = idx;
                            }
                        }

                        EeSpinBox {
                            id: volume

                            label: i18n("Volume")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("volume")
                            to: pluginDB.getMaxValue("volume")
                            value: pluginDB.volume
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                pluginDB.volume = v;
                            }
                        }

                        EeSpinBox {
                            id: clippingRange

                            label: i18n("Clipping Range")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("clippingRange")
                            to: pluginDB.getMaxValue("clippingRange")
                            value: pluginDB.clippingRange
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: pluginDB.clipping
                            onValueModified: v => {
                                pluginDB.clippingRange = v;
                            }
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: loudnessPage.pluginDB
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
                    text: i18n("Clipping")
                    icon.name: "path-cut-symbolic"
                    checkable: true
                    checked: pluginDB.clipping
                    onTriggered: {
                        if (pluginDB.clipping != checked)
                            pluginDB.clipping = checked;
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
