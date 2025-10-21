import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: levelMeterPage

    required property string name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        momentary.value = pluginBackend.getMomentaryLevel();
        shortterm.value = pluginBackend.getShorttermLevel();
        integrated.value = pluginBackend.getIntegratedLevel();
        relative.value = pluginBackend.getRelativeLevel();
        range.value = pluginBackend.getRangeLevel();
        truePeakL.value = pluginBackend.getTruePeakL();
        truePeakR.value = pluginBackend.getTruePeakR();
        inputL.value = pluginBackend.getInputLevelLeft();
        inputR.value = pluginBackend.getInputLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            Layout.fillWidth: true
            uniformCellWidths: true

            ColumnLayout {
                Kirigami.Card {
                    id: cardInputLevels

                    header: Kirigami.Heading {
                        text: i18n("Input Level") // qmllint disable
                        level: 2
                    }

                    contentItem: Column {
                        spacing: Kirigami.Units.gridUnit

                        EeProgressBar {
                            id: inputL

                            label: i18n("Left") // qmllint disable
                            unit: "dB"
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1

                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                        }

                        EeProgressBar {
                            id: inputR

                            label: i18n("Right") // qmllint disable
                            unit: "dB"
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1

                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                        }
                    }
                }

                Kirigami.Card {
                    id: cardPeak

                    header: Kirigami.Heading {
                        text: i18n("True Peak") // qmllint disable
                        level: 2
                    }

                    contentItem: Column {
                        spacing: Kirigami.Units.gridUnit

                        EeProgressBar {
                            id: truePeakL

                            label: i18n("Left") // qmllint disable
                            unit: "dB"
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1

                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                        }

                        EeProgressBar {
                            id: truePeakR

                            label: i18n("Right") // qmllint disable
                            unit: "dB"
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1

                            anchors {
                                left: parent.left
                                right: parent.right
                            }
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLoudness

                header: Kirigami.Heading {
                    text: i18n("Loudness") // qmllint disable
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeProgressBar {
                        id: momentary

                        label: i18n("Momentary") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: shortterm

                        label: i18n("Short-Term") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: integrated

                        label: i18n("Integrated") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: relative

                        label: i18n("Relative") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }

                    EeProgressBar {
                        id: range

                        label: i18n("Range") // qmllint disable
                        unit: "LU"
                        from: 0
                        to: 50
                        value: 0
                        decimals: 1

                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                    }
                }
            }
        }
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.ebur128}</strong>`) // qmllint disable
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
                    text: i18n("Reset History") // qmllint disable
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        levelMeterPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        levelMeterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
