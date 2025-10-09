import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
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
                        id: loudness

                        label: i18n("Loudness")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("loudness")
                        to: pluginDB.getMaxValue("loudness")
                        value: pluginDB.loudness
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.loudness = v;
                        }
                    }

                    EeSpinBox {
                        id: output

                        label: i18n("Output")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("output")
                        to: pluginDB.getMaxValue("output")
                        value: pluginDB.output
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.output = v;
                        }
                    }

                    EeSpinBox {
                        id: link

                        label: i18n("Link")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("link")
                        to: pluginDB.getMaxValue("link")
                        value: pluginDB.link
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.link = v;
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.mda}</b>`)
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
