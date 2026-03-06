import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: immersiveSurroundPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.setInputLevelLeft(pluginBackend.getInputLevelLeft());
        inputOutputLevels.setInputLevelRight(pluginBackend.getInputLevelRight());
        inputOutputLevels.setOutputLevelLeft(pluginBackend.getOutputLevelLeft());
        inputOutputLevels.setOutputLevelRight(pluginBackend.getOutputLevelRight());
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true

            EeCard {
                title: i18n("Spatial") // qmllint disable

                EeSpinBox {
                    id: surroundAmount

                    label: i18n("Surround amount") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("surroundAmount")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("surroundAmount")
                    value: immersiveSurroundPage.pluginDB.surroundAmount
                    decimals: 0
                    stepSize: 1
                    unit: Units.percent
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.surroundAmount = v;
                    }
                }

                EeSpinBox {
                    id: stereoWidth

                    label: i18n("Stereo width") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("stereoWidth")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("stereoWidth")
                    value: immersiveSurroundPage.pluginDB.stereoWidth
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.stereoWidth = v;
                    }
                }

                EeSpinBox {
                    id: rearDelay

                    label: i18n("Rear delay") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("rearDelay")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("rearDelay")
                    value: immersiveSurroundPage.pluginDB.rearDelay
                    decimals: 1
                    stepSize: 0.1
                    unit: Units.ms
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.rearDelay = v;
                    }
                }

                EeSpinBox {
                    id: rearMix

                    label: i18n("Rear mix") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("rearMix")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("rearMix")
                    value: immersiveSurroundPage.pluginDB.rearMix
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.rearMix = v;
                    }
                }
            }

            EeCard {
                title: i18n("Tuning") // qmllint disable

                EeSpinBox {
                    id: centerLevel

                    label: i18n("Center level") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("centerLevel")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("centerLevel")
                    value: immersiveSurroundPage.pluginDB.centerLevel
                    decimals: 2
                    stepSize: 0.01
                    unit: Units.dB
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.centerLevel = v;
                    }
                }

                EeSpinBox {
                    id: damping

                    label: i18n("Rear damping") // qmllint disable
                    spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                    from: immersiveSurroundPage.pluginDB.getMinValue("damping")
                    to: immersiveSurroundPage.pluginDB.getMaxValue("damping")
                    value: immersiveSurroundPage.pluginDB.damping
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        immersiveSurroundPage.pluginDB.damping = v;
                    }
                }

                EeSwitch {
                    id: rearPhaseInvert

                    label: i18n("Invert rear phase") // qmllint disable
                    isChecked: immersiveSurroundPage.pluginDB.rearPhaseInvert
                    onCheckedChanged: {
                        if (isChecked !== immersiveSurroundPage.pluginDB.rearPhaseInvert)
                            immersiveSurroundPage.pluginDB.rearPhaseInvert = isChecked;
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: immersiveSurroundPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.ee}</strong>`) // qmllint disable
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
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        immersiveSurroundPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
