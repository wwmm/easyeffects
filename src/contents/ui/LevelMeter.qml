import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
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
                        leftPadding: Kirigami.Units.largeSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                    }

                    contentItem: ColumnLayout {
                        spacing: 0

                        EeProgressBar {
                            id: inputL

                            label: i18n("Left") // qmllint disable
                            unit: i18n("dB")
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1
                        }

                        EeProgressBar {
                            id: inputR

                            label: i18n("Right") // qmllint disable
                            unit: i18n("dB")
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }

                Kirigami.Card {
                    id: cardPeak

                    header: Kirigami.Heading {
                        text: i18n("True Peak") // qmllint disable
                        level: 2
                        leftPadding: Kirigami.Units.largeSpacing
                        rightPadding: Kirigami.Units.largeSpacing
                    }

                    contentItem: ColumnLayout {
                        spacing: 0

                        EeProgressBar {
                            id: truePeakL

                            label: i18n("Left") // qmllint disable
                            unit: i18n("dB")
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1
                        }

                        EeProgressBar {
                            id: truePeakR

                            label: i18n("Right") // qmllint disable
                            unit: i18n("dB")
                            from: Common.minimumDecibelLevel
                            to: 10
                            value: 0
                            decimals: 1
                        }

                        Item {
                            Layout.fillHeight: true
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLoudness

                header: Kirigami.Heading {
                    text: i18n("Loudness") // qmllint disable
                    level: 2
                    leftPadding: Kirigami.Units.largeSpacing
                    rightPadding: Kirigami.Units.largeSpacing
                }

                contentItem: ColumnLayout {
                    spacing: 0

                    EeProgressBar {
                        id: momentary

                        label: i18n("Momentary") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1
                    }

                    EeProgressBar {
                        id: shortterm

                        label: i18n("Short-term") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1
                    }

                    EeProgressBar {
                        id: integrated

                        label: i18n("Integrated") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1
                    }

                    EeProgressBar {
                        id: relative

                        label: i18n("Relative") // qmllint disable
                        unit: "LUFS"
                        from: Common.minimumDecibelLevel
                        to: 10
                        value: 0
                        decimals: 1
                    }

                    EeProgressBar {
                        id: range

                        label: i18n("Range") // qmllint disable
                        unit: "LU"
                        from: 0
                        to: 50
                        value: 0
                        decimals: 1
                    }

                    Item {
                        Layout.fillHeight: true
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
                    text: i18n("Reset history") // qmllint disable
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        levelMeterPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        levelMeterPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
