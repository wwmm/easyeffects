import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: maximizerPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = maximizerPage.pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = maximizerPage.pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = maximizerPage.pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = maximizerPage.pluginBackend.getOutputLevelRight();
        reductionLevel.value = maximizerPage.pluginBackend.getReductionLevel();
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
                    EeSpinBox {
                        id: release

                        label: i18n("Release") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: maximizerPage.pluginDB.getMinValue("release")
                        to: maximizerPage.pluginDB.getMaxValue("release")
                        value: maximizerPage.pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: v => {
                            maximizerPage.pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: threshold

                        label: i18n("Threshold") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: maximizerPage.pluginDB.getMinValue("threshold")
                        to: maximizerPage.pluginDB.getMaxValue("threshold")
                        value: maximizerPage.pluginDB.threshold
                        decimals: 2
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            maximizerPage.pluginDB.threshold = v;
                        }
                    }

                    EeProgressBar {
                        id: reductionLevel
                        Layout.topMargin: Kirigami.Units.largeSpacing

                        label: i18n("Reduction") // qmllint disable
                        unit: "dB"
                        from: 0
                        to: 40
                        value: 0
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.zam}</b>`) // qmllint disable
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
                    checked: maximizerPage.pluginBackend ? maximizerPage.pluginBackend.hasNativeUi() : false
                    onTriggered: {
                        if (checked)
                            maximizerPage.pluginBackend.showNativeUi();
                        else
                            maximizerPage.pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        maximizerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
