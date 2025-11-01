import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigami.layouts as KirigamiLayouts
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: loudnessPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = loudnessPage.pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = loudnessPage.pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = loudnessPage.pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = loudnessPage.pluginBackend.getOutputLevelRight();
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
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            text: i18n("Mode") // qmllint disable
                            Layout.columnSpan: 2
                            Layout.alignment: Qt.AlignHCenter
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: loudnessPage.pluginDB.mode
                            editable: false
                            model: ["FFT", "IIR"]
                            onActivated: idx => {
                                loudnessPage.pluginDB.mode = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: std

                            text: i18n("Contour") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: loudnessPage.pluginDB.std
                            editable: false
                            model: ["Flat", "ISO226-2003", "Fletcher-Munson", "Robinson-Dadson", "ISO226-2023"]
                            onActivated: idx => {
                                loudnessPage.pluginDB.std = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: fft

                            text: i18n("FFT size") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: loudnessPage.pluginDB.fft
                            editable: false
                            model: [256, 512, 1024, 2048, 4096, 8192, 16384]
                            visible: loudnessPage.pluginDB.mode == 0 ? true : false
                            onActivated: idx => {
                                loudnessPage.pluginDB.fft = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: iirApproximation

                            text: i18n("IIR approximation") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: loudnessPage.pluginDB.iirApproximation
                            editable: false
                            model: [i18n("Fastest"), i18n("Low"), i18n("Normal"), i18n("High"), i18n("Best")]// qmllint disable
                            visible: loudnessPage.pluginDB.mode == 1 ? true : false
                            onActivated: idx => {
                                loudnessPage.pluginDB.iirApproximation = idx;
                            }
                        }

                        EeSpinBox {
                            id: volume

                            label: i18n("Volume") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: loudnessPage.pluginDB.getMinValue("volume")
                            to: loudnessPage.pluginDB.getMaxValue("volume")
                            value: loudnessPage.pluginDB.volume
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                loudnessPage.pluginDB.volume = v;
                            }
                        }

                        EeSpinBox {
                            id: clippingRange

                            label: i18n("Clipping range") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: loudnessPage.pluginDB.getMinValue("clippingRange")
                            to: loudnessPage.pluginDB.getMaxValue("clippingRange")
                            value: loudnessPage.pluginDB.clippingRange
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            enabled: loudnessPage.pluginDB.clipping
                            onValueModified: v => {
                                loudnessPage.pluginDB.clippingRange = v;
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.lsp}</b>`) // qmllint disable
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
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Show native window") // qmllint disable
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: loudnessPage.pluginBackend ? loudnessPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            loudnessPage.pluginBackend.showNativeUi();
                        else
                            loudnessPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Clipping") // qmllint disable
                    icon.name: "path-cut-symbolic"
                    checkable: true
                    checked: loudnessPage.pluginDB.clipping
                    onTriggered: {
                        if (loudnessPage.pluginDB.clipping != checked)
                            loudnessPage.pluginDB.clipping = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: KirigamiLayouts.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        loudnessPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
