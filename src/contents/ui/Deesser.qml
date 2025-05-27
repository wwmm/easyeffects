import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: deesserPage

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
        detectionLevel.value = pluginBackend.getDetectedLevel();
        gainReduction.value = pluginBackend.getCompressionLevel();
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
                    text: i18n("Deesser")
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: detection

                            text: i18n("Detection")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.detection
                            editable: false
                            model: [i18n("RMS"), i18n("Peak")]
                            onActivated: (idx) => {
                                pluginDB.detection = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.mode
                            editable: false
                            model: [i18n("Wide"), i18n("Split")]
                            onActivated: (idx) => {
                                pluginDB.mode = idx;
                            }
                        }

                        EeSpinBox {
                            id: threshold

                            label: i18n("Threshold")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("threshold")
                            to: pluginDB.getMaxValue("threshold")
                            value: pluginDB.threshold
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: (v) => {
                                pluginDB.threshold = v;
                            }
                        }

                        EeSpinBox {
                            id: ratio

                            label: i18n("Ratio")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("ratio")
                            to: pluginDB.getMaxValue("ratio")
                            value: pluginDB.ratio
                            decimals: 0
                            stepSize: 1
                            onValueModified: (v) => {
                                pluginDB.ratio = v;
                            }
                        }

                        EeSpinBox {
                            id: makeup

                            label: i18n("Makeup")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("makeup")
                            to: pluginDB.getMaxValue("makeup")
                            value: pluginDB.makeup
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: (v) => {
                                pluginDB.makeup = v;
                            }
                        }

                        EeSpinBox {
                            id: laxity

                            label: i18n("Laxity")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("laxity")
                            to: pluginDB.getMaxValue("laxity")
                            value: pluginDB.laxity
                            decimals: 0
                            stepSize: 1
                            onValueModified: (v) => {
                                pluginDB.laxity = v;
                            }
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
                            id: f1Freq

                            label: i18n("F1 Split")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("f1Freq")
                            to: pluginDB.getMaxValue("f1Freq")
                            value: pluginDB.f1Freq
                            decimals: 2
                            stepSize: 0.01
                            unit: "Hz"
                            onValueModified: (v) => {
                                pluginDB.f1Freq = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Freq

                            label: i18n("F2 Peak")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("f2Freq")
                            to: pluginDB.getMaxValue("f2Freq")
                            value: pluginDB.f2Freq
                            decimals: 2
                            stepSize: 0.01
                            unit: "Hz"
                            onValueModified: (v) => {
                                pluginDB.f2Freq = v;
                            }
                        }

                        EeSpinBox {
                            id: f1Level

                            label: i18n("F1 Gain")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("f1Level")
                            to: pluginDB.getMaxValue("f1Level")
                            value: pluginDB.f1Level
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: (v) => {
                                pluginDB.f1Level = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Level

                            label: i18n("F2 Level")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("f2Level")
                            to: pluginDB.getMaxValue("f2Level")
                            value: pluginDB.f2Level
                            decimals: 1
                            stepSize: 0.1
                            unit: "dBFS"
                            onValueModified: (v) => {
                                pluginDB.f2Level = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Q

                            Layout.columnSpan: 2
                            label: i18n("F2 Peak Q")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("f2Q")
                            to: pluginDB.getMaxValue("f2Q")
                            value: pluginDB.f2Q
                            decimals: 3
                            stepSize: 0.001
                            onValueModified: (v) => {
                                pluginDB.f2Q = v;
                            }
                        }

                    }

                }

            }

            Kirigami.Card {
                id: cardDetectionLevels

                Layout.columnSpan: 2

                header: Kirigami.Heading {
                    text: i18n("Level")
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeProgressBar {
                        id: detectionLevel

                        label: i18n("Detection")
                        unit: "dB"
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
                        id: gainReduction

                        label: i18n("Gain Reduction")
                        unit: "dB"
                        from: Common.minimumDecibelLevel
                        to: 10
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

        pluginDB: deesserPage.pluginDB
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
                    text: i18n("Listen")
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: pluginDB.scListen
                    onTriggered: {
                        if (pluginDB.scListen != checked)
                            pluginDB.scListen = checked;

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
