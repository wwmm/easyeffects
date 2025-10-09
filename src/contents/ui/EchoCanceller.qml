import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: echoCancellerPage

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

            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: filterLength

                        label: i18n("Filter Length")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("filterLength")
                        to: pluginDB.getMaxValue("filterLength")
                        value: pluginDB.filterLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            pluginDB.filterLength = v;
                        }
                    }

                    EeSpinBox {
                        id: residualEchoSuppression

                        label: i18n("Residual Echo Suppression")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("residualEchoSuppression")
                        to: pluginDB.getMaxValue("residualEchoSuppression")
                        value: pluginDB.residualEchoSuppression
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.residualEchoSuppression = v;
                        }
                    }

                    EeSpinBox {
                        id: nearEndSuppression

                        label: i18n("Near End Suppression")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("nearEndSuppression")
                        to: pluginDB.getMaxValue("nearEndSuppression")
                        value: pluginDB.nearEndSuppression
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.nearEndSuppression = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: echoCancellerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.speex}</b>`)
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
