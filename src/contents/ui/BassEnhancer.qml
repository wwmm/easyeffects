import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: bassEnchancerPage

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
        harmonicsLevel.value = pluginBackend.getHarmonicsLevel();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Blend Harmonics")
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 2 * cardLayout.maximumColumnWidth

            Controls.Label {
                Layout.alignment: Qt.AlignLeft
                text: i18n("3rd")
            }

            Controls.Slider {
                id: blend

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                value: pluginDB.blend
                from: pluginDB.getMinValue("blend")
                to: pluginDB.getMaxValue("blend")
                stepSize: 1
                onValueChanged: () => {
                    if (value !== pluginDB.blend)
                        pluginDB.blend = value;
                }
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: i18n("2nd")
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: blend.value
        }

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: amount

                        label: i18n("Amount")
                        from: pluginDB.getMinValue("amount")
                        to: pluginDB.getMaxValue("amount")
                        value: pluginDB.amount
                        decimals: 2
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.amount = v;
                        }
                    }

                    EeSpinBox {
                        id: harmonics

                        label: i18n("Harmonics")
                        from: pluginDB.getMinValue("harmonics")
                        to: pluginDB.getMaxValue("harmonics")
                        value: pluginDB.harmonics
                        decimals: 1
                        stepSize: 0.1
                        onValueModified: v => {
                            pluginDB.harmonics = v;
                        }
                    }

                    EeSpinBox {
                        id: scope

                        label: i18n("Scope")
                        from: pluginDB.getMinValue("scope")
                        to: pluginDB.getMaxValue("scope")
                        value: pluginDB.scope
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        onValueModified: v => {
                            pluginDB.scope = v;
                        }
                    }

                    EeSwitch {
                        id: floorActive

                        label: i18n("Floor Active")
                        isChecked: pluginDB.floorActive
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.floorActive)
                                pluginDB.floorActive = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: floor

                        label: i18n("Floor")
                        from: pluginDB.getMinValue("floor")
                        to: pluginDB.getMaxValue("floor")
                        value: pluginDB.floor
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        enabled: floorActive.isChecked
                        onValueModified: v => {
                            pluginDB.floor = v;
                        }
                    }

                    EeProgressBar {
                        id: harmonicsLevel
                        Layout.topMargin: Kirigami.Units.largeSpacing

                        label: i18n("Harmonics")
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 0
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: bassEnchancerPage.pluginDB
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
                    checked: pluginDB.listen
                    onTriggered: {
                        if (pluginDB.listen != checked)
                            pluginDB.listen = checked;
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
