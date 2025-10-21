import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: deesserPage

    required property string name
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
        detectionLevel.value = pluginBackend.getDetectedLevel();
        gainReduction.value = pluginBackend.getCompressionLevel();
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
                    text: i18n("Deesser") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        FormCard.FormComboBoxDelegate {
                            id: detection

                            text: i18n("Detection") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: deesserPage.pluginDB.detection
                            editable: false
                            model: [i18n("RMS"), i18n("Peak")]// qmllint disable
                            onActivated: idx => {
                                deesserPage.pluginDB.detection = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            text: i18n("Mode") // qmllint disable
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: deesserPage.pluginDB.mode
                            editable: false
                            model: [i18n("Wide"), i18n("Split")]// qmllint disable
                            onActivated: idx => {
                                deesserPage.pluginDB.mode = idx;
                            }
                        }

                        EeSpinBox {
                            id: threshold

                            label: i18n("Threshold") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("threshold")
                            to: deesserPage.pluginDB.getMaxValue("threshold")
                            value: deesserPage.pluginDB.threshold
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                deesserPage.pluginDB.threshold = v;
                            }
                        }

                        EeSpinBox {
                            id: ratio

                            label: i18n("Ratio") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("ratio")
                            to: deesserPage.pluginDB.getMaxValue("ratio")
                            value: deesserPage.pluginDB.ratio
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                deesserPage.pluginDB.ratio = v;
                            }
                        }

                        EeSpinBox {
                            id: makeup

                            label: i18n("Makeup") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("makeup")
                            to: deesserPage.pluginDB.getMaxValue("makeup")
                            value: deesserPage.pluginDB.makeup
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                deesserPage.pluginDB.makeup = v;
                            }
                        }

                        EeSpinBox {
                            id: laxity

                            label: i18n("Laxity") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("laxity")
                            to: deesserPage.pluginDB.getMaxValue("laxity")
                            value: deesserPage.pluginDB.laxity
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                deesserPage.pluginDB.laxity = v;
                            }
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
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: f1Freq

                            label: i18n("F1 Split") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("f1Freq")
                            to: deesserPage.pluginDB.getMaxValue("f1Freq")
                            value: deesserPage.pluginDB.f1Freq
                            decimals: 2
                            stepSize: 0.01
                            unit: "Hz"
                            onValueModified: v => {
                                deesserPage.pluginDB.f1Freq = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Freq

                            label: i18n("F2 Peak") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("f2Freq")
                            to: deesserPage.pluginDB.getMaxValue("f2Freq")
                            value: deesserPage.pluginDB.f2Freq
                            decimals: 2
                            stepSize: 0.01
                            unit: "Hz"
                            onValueModified: v => {
                                deesserPage.pluginDB.f2Freq = v;
                            }
                        }

                        EeSpinBox {
                            id: f1Level

                            label: i18n("F1 Gain") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("f1Level")
                            to: deesserPage.pluginDB.getMaxValue("f1Level")
                            value: deesserPage.pluginDB.f1Level
                            decimals: 1
                            stepSize: 0.1
                            unit: "dB"
                            onValueModified: v => {
                                deesserPage.pluginDB.f1Level = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Level

                            label: i18n("F2 Level") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("f2Level")
                            to: deesserPage.pluginDB.getMaxValue("f2Level")
                            value: deesserPage.pluginDB.f2Level
                            decimals: 1
                            stepSize: 0.1
                            unit: "dBFS"
                            onValueModified: v => {
                                deesserPage.pluginDB.f2Level = v;
                            }
                        }

                        EeSpinBox {
                            id: f2Q

                            Layout.columnSpan: 2
                            label: i18n("F2 Peak Q") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: deesserPage.pluginDB.getMinValue("f2Q")
                            to: deesserPage.pluginDB.getMaxValue("f2Q")
                            value: deesserPage.pluginDB.f2Q
                            decimals: 3
                            stepSize: 0.001
                            onValueModified: v => {
                                deesserPage.pluginDB.f2Q = v;
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Kirigami.Card {
                id: cardDetectionLevels

                Layout.topMargin: Kirigami.Units.smallSpacing
                header: Kirigami.Heading {
                    text: i18n("Level") // qmllint disable
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeProgressBar {
                        id: detectionLevel

                        label: i18n("Detection") // qmllint disable
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

                        label: i18n("Gain Reduction") // qmllint disable
                        unit: "dB"
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 0
                        decimals: 1
                        rightToLeft: true

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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.calf}</strong>`) // qmllint disable
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
                    checked: deesserPage.pluginBackend ? deesserPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            deesserPage.pluginBackend.showNativeUi();
                        else
                            deesserPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Listen") // qmllint disable
                    icon.name: "audio-headset-symbolic"
                    checkable: true
                    checked: deesserPage.pluginDB.scListen
                    onTriggered: {
                        if (deesserPage.pluginDB.scListen != checked)
                            deesserPage.pluginDB.scListen = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        deesserPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
