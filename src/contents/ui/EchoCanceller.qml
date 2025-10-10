import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
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
                    text: i18n("Controls") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: filterLength

                        label: i18n("Filter Length") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: echoCancellerPage.pluginDB.getMinValue("filterLength")
                        to: echoCancellerPage.pluginDB.getMaxValue("filterLength")
                        value: echoCancellerPage.pluginDB.filterLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            echoCancellerPage.pluginDB.filterLength = v;
                        }
                    }

                    EeSpinBox {
                        id: residualEchoSuppression

                        label: i18n("Residual Echo Suppression") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: echoCancellerPage.pluginDB.getMinValue("residualEchoSuppression")
                        to: echoCancellerPage.pluginDB.getMaxValue("residualEchoSuppression")
                        value: echoCancellerPage.pluginDB.residualEchoSuppression
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: v => {
                            echoCancellerPage.pluginDB.residualEchoSuppression = v;
                        }
                    }

                    EeSpinBox {
                        id: nearEndSuppression

                        label: i18n("Near End Suppression") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: echoCancellerPage.pluginDB.getMinValue("nearEndSuppression")
                        to: echoCancellerPage.pluginDB.getMaxValue("nearEndSuppression")
                        value: echoCancellerPage.pluginDB.nearEndSuppression
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: v => {
                            echoCancellerPage.pluginDB.nearEndSuppression = v;
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.speex}</b>`) // qmllint disable
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
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        echoCancellerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
