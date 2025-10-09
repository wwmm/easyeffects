import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: speexPage

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

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSwitch {
                        id: enableDenoise

                        label: i18n("Denoise")
                        isChecked: pluginDB.enableDenoise
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.enableDenoise)
                                pluginDB.enableDenoise = isChecked;
                        }
                    }

                    EeSwitch {
                        id: enableAgc

                        label: i18n("Automatic Gain Control")
                        isChecked: pluginDB.enableAgc
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.enableAgc)
                                pluginDB.enableAgc = isChecked;
                        }
                    }

                    EeSwitch {
                        id: enableDereverb

                        label: i18n("Dereverberation")
                        isChecked: pluginDB.enableDereverb
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.enableDereverb)
                                pluginDB.enableDereverb = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: noiseSuppression

                        label: i18n("Noise Suppression")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("noiseSuppression")
                        to: pluginDB.getMaxValue("noiseSuppression")
                        value: pluginDB.noiseSuppression
                        decimals: 2
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.noiseSuppression = v;
                        }
                    }
                }
            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Voice Activity Probability")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSwitch {
                        id: enableVad

                        label: i18n("Voice Detection")
                        isChecked: pluginDB.enableVad
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.enableVad)
                                pluginDB.enableVad = isChecked;
                        }
                    }

                    EeSpinBox {
                        id: vadProbabilityStart

                        Layout.columnSpan: 2
                        label: i18n("Start")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("vadProbabilityStart")
                        to: pluginDB.getMaxValue("vadProbabilityStart")
                        value: pluginDB.vadProbabilityStart
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        enabled: enableVad.isChecked
                        onValueModified: v => {
                            pluginDB.vadProbabilityStart = v;
                        }
                    }

                    EeSpinBox {
                        id: vadProbabilityContinue

                        Layout.columnSpan: 2
                        label: i18n("Continue")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("vadProbabilityContinue")
                        to: pluginDB.getMaxValue("vadProbabilityContinue")
                        value: pluginDB.vadProbabilityContinue
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        enabled: enableVad.isChecked
                        onValueModified: v => {
                            pluginDB.vadProbabilityContinue = v;
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: speexPage.pluginDB
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
