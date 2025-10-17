import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

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
            maximumColumns: 3
            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Echo Canceller") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSwitch {
                        label: i18n("Enable") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.enableEchoCanceller
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.enableEchoCanceller)
                                echoCancellerPage.pluginDB.enableEchoCanceller = isChecked;
                        }
                    }

                    EeSwitch {
                        label: i18n("Mobile Mode") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.echoCancellerMobileMode
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.echoCancellerMobileMode)
                                echoCancellerPage.pluginDB.echoCancellerMobileMode = isChecked;
                        }
                    }

                    EeSwitch {
                        label: i18n("Enforce High-pass") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.echoCancellerEnforceHighPass
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.echoCancellerEnforceHighPass)
                                echoCancellerPage.pluginDB.echoCancellerEnforceHighPass = isChecked;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardNoiseSuppression

                header: Kirigami.Heading {
                    text: i18n("Noise Suppression") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSwitch {
                        label: i18n("Enable") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.enableNoiseSuppression
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.enableNoiseSuppression)
                                echoCancellerPage.pluginDB.enableNoiseSuppression = isChecked;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        text: i18n("Level") // qmllint disable
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: echoCancellerPage.pluginDB.noiseSuppressionLevel
                        editable: false
                        model: [i18n("Low"), i18n("Moderate"), i18n("High"), i18n("Very High")]
                        onActivated: idx => {
                            echoCancellerPage.pluginDB.noiseSuppressionLevel = idx;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardHighPass

                header: Kirigami.Heading {
                    text: i18n("High-pass Filter") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors.fill: parent

                    EeSwitch {
                        label: i18n("Enable") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.enableHighPassFilter
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.enableHighPassFilter)
                                echoCancellerPage.pluginDB.enableHighPassFilter = isChecked;
                        }
                    }

                    EeSwitch {
                        label: i18n("Full Band") // qmllint disable
                        isChecked: echoCancellerPage.pluginDB.highPassFilterFullBand
                        onCheckedChanged: {
                            if (isChecked !== echoCancellerPage.pluginDB.highPassFilterFullBand)
                                echoCancellerPage.pluginDB.highPassFilterFullBand = isChecked;
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.webrtc}</b>`) // qmllint disable
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
                    text: i18n("Automatic Gain Control") // qmllint disable
                    icon.name: "auto-scale-y"
                    checkable: true
                    checked: echoCancellerPage.pluginDB.enableAGC
                    onTriggered: {
                        if (echoCancellerPage.pluginDB.enableAGC != checked)
                            echoCancellerPage.pluginDB.enableAGC = checked;
                    }
                },
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
