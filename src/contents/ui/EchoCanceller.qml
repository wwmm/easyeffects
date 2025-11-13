import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: echoCancellerPage

    required property string name
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
            maximumColumns: 3
            uniformCellWidths: true

            EeCard {
                id: cardControls

                title: i18n("Echo Canceller") // qmllint disable

                EeSwitch {
                    label: i18n("Enable") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.enableEchoCanceller
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.enableEchoCanceller)
                            echoCancellerPage.pluginDB.enableEchoCanceller = isChecked;
                    }
                }

                EeSwitch {
                    label: i18n("Mobile mode") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.echoCancellerMobileMode
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.echoCancellerMobileMode)
                            echoCancellerPage.pluginDB.echoCancellerMobileMode = isChecked;
                    }
                }

                EeSwitch {
                    label: i18n("Enforce high-pass") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.echoCancellerEnforceHighPass
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.echoCancellerEnforceHighPass)
                            echoCancellerPage.pluginDB.echoCancellerEnforceHighPass = isChecked;
                    }
                }
            }

            EeCard {
                id: cardNoiseSuppression

                title: i18n("Noise Suppression") // qmllint disable

                EeSwitch {
                    label: i18n("Enable") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.enableNoiseSuppression
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.enableNoiseSuppression)
                            echoCancellerPage.pluginDB.enableNoiseSuppression = isChecked;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    verticalPadding: Kirigami.Units.smallUnit
                    text: i18n("Strength") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: echoCancellerPage.pluginDB.noiseSuppressionLevel
                    editable: false
                    model: [i18n("Low"), i18n("Moderate"), i18n("High"), i18n("Very high")]
                    onActivated: idx => {
                        echoCancellerPage.pluginDB.noiseSuppressionLevel = idx;
                    }
                }
            }

            EeCard {
                id: cardHighPass

                title: i18n("High-Pass Filter") // qmllint disable

                EeSwitch {
                    label: i18n("Enable") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.enableHighPassFilter
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.enableHighPassFilter)
                            echoCancellerPage.pluginDB.enableHighPassFilter = isChecked;
                    }
                }

                EeSwitch {
                    label: i18n("Full band") // qmllint disable
                    isChecked: echoCancellerPage.pluginDB.highPassFilterFullBand
                    onCheckedChanged: {
                        if (isChecked !== echoCancellerPage.pluginDB.highPassFilterFullBand)
                            echoCancellerPage.pluginDB.highPassFilterFullBand = isChecked;
                    }
                }

                Item {
                    Layout.fillHeight: true
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.webrtc}</b>`) // qmllint disable
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
                    text: i18n("Automatic gain control") // qmllint disable
                    icon.name: "auto-scale-y"
                    checkable: true
                    checked: echoCancellerPage.pluginDB.enableAGC
                    onTriggered: {
                        if (echoCancellerPage.pluginDB.enableAGC != checked)
                            echoCancellerPage.pluginDB.enableAGC = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
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
