import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: deepfilternetPage

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
        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Attenuation Limit")
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 2 * cardLayout.maximumColumnWidth

            Controls.Slider {
                id: attenuationLimit

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                from: pluginDB.getMinValue("attenuationLimit")
                to: pluginDB.getMaxValue("attenuationLimit")
                value: pluginDB.attenuationLimit
                stepSize: 1
                onValueChanged: () => {
                    if (value !== pluginDB.attenuationLimit)
                        pluginDB.attenuationLimit = value;
                }
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: attenuationLimit.value
        }

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
                    EeSpinBox {
                        id: minProcessingThreshold

                        label: i18n("Minimum Processing Threshold")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: pluginDB.getMinValue("minProcessingThreshold")
                        to: pluginDB.getMaxValue("minProcessingThreshold")
                        value: pluginDB.minProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.minProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: maxErbProcessingThreshold

                        label: i18n("Maximum ERB Processing threshold")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: pluginDB.getMinValue("maxErbProcessingThreshold")
                        to: pluginDB.getMaxValue("maxErbProcessingThreshold")
                        value: pluginDB.maxErbProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.maxErbProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: maxDfProcessingThreshold

                        label: i18n("Maximum DF Processing Threshold")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: pluginDB.getMinValue("maxDfProcessingThreshold")
                        to: pluginDB.getMaxValue("maxDfProcessingThreshold")
                        value: pluginDB.maxDfProcessingThreshold
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.maxDfProcessingThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: minProcessingBuffer

                        label: i18n("Minimum Processing Buffer")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: pluginDB.getMinValue("minProcessingBuffer")
                        to: pluginDB.getMaxValue("minProcessingBuffer")
                        value: pluginDB.minProcessingBuffer
                        decimals: 0
                        stepSize: 1
                        unit: i18n("frames")
                        onValueModified: v => {
                            pluginDB.minProcessingBuffer = v;
                        }
                    }

                    EeSpinBox {
                        id: postFilterBeta

                        label: i18n("Post Filter Beta")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 8
                        from: pluginDB.getMinValue("postFilterBeta")
                        to: pluginDB.getMaxValue("postFilterBeta")
                        value: pluginDB.postFilterBeta
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.postFilterBeta = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: deepfilternetPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.deepfilternet}</b>`)
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
                    text: i18n("Reset History")
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        pluginBackend.resetHistory();
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
