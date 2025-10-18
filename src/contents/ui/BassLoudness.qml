import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: bassLoudnessPage

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
                    text: i18n("Controls")// qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: loudness

                        label: i18n("Loudness")// qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: bassLoudnessPage.pluginDB.getMinValue("loudness")
                        to: bassLoudnessPage.pluginDB.getMaxValue("loudness")
                        value: bassLoudnessPage.pluginDB.loudness
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            bassLoudnessPage.pluginDB.loudness = v;
                        }
                    }

                    EeSpinBox {
                        id: output

                        label: i18n("Output")// qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: bassLoudnessPage.pluginDB.getMinValue("output")
                        to: bassLoudnessPage.pluginDB.getMaxValue("output")
                        value: bassLoudnessPage.pluginDB.output
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            bassLoudnessPage.pluginDB.output = v;
                        }
                    }

                    EeSpinBox {
                        id: link

                        label: i18n("Link")// qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: bassLoudnessPage.pluginDB.getMinValue("link")
                        to: bassLoudnessPage.pluginDB.getMaxValue("link")
                        value: bassLoudnessPage.pluginDB.link
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            bassLoudnessPage.pluginDB.link = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: bassLoudnessPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.mda}</b>`)// qmllint disable
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
                        bassLoudnessPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
