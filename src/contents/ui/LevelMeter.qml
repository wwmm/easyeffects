import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: levelMeterPage

    required property var name
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
                        text: i18n("Input Level")
                        level: 2
                    }

                    contentItem: Column {
                        spacing: Kirigami.Units.gridUnit

                        EeProgressBar {
                            id: inputL

                            label: i18n("Left")
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

                            label: i18n("Right")
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
                        text: i18n("True Peak")
                        level: 2
                    }

                    contentItem: Column {
                        spacing: Kirigami.Units.gridUnit

                        EeProgressBar {
                            id: truePeakL

                            label: i18n("Left")
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

                            label: i18n("Right")
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
                    text: i18n("Loudness")
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeProgressBar {
                        id: momentary

                        label: i18n("Momentary")
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

                        label: i18n("Short-Term")
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

                        label: i18n("Integrated")
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

                        label: i18n("Relative")
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

                        label: i18n("Range")
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.ebur128}</b>`)
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
                    text: i18n("Reset History")
                    icon.name: "edit-clear-history-symbolic"
                    onTriggered: {
                        pluginBackend.resetHistory();
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
