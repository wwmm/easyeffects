import "Common.js" as Common
import EEdbm
import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: maximizerPage

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
        reductionLevel.value = pluginBackend.getReductionLevel();
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
                    EeSpinBox {
                        id: release

                        label: i18n("Release")
                        from: pluginDB.getMinValue("release")
                        to: pluginDB.getMaxValue("release")
                        value: pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: threshold

                        label: i18n("Threshold")
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

                    EeProgressBar {
                        id: reductionLevel

                        label: i18n("Reduction")
                        unit: i18n("dB")
                        from: Common.minimumDecibelLevel
                        to: 0
                        value: 40
                        decimals: 0
                    }

                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: maximizerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.zam
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
