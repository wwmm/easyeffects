import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: pitchPage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return ;

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
            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Quality")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: sequenceLength

                        label: i18n("Sequence Length")
                        from: pluginDB.getMinValue("sequenceLength")
                        to: pluginDB.getMaxValue("sequenceLength")
                        value: pluginDB.sequenceLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.sequenceLength = v;
                        }
                    }

                    EeSpinBox {
                        id: seekWindow

                        label: i18n("Seek Window")
                        from: pluginDB.getMinValue("seekWindow")
                        to: pluginDB.getMaxValue("seekWindow")
                        value: pluginDB.seekWindow
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.seekWindow = v;
                        }
                    }

                    EeSpinBox {
                        id: overlapLength

                        label: i18n("OVerlap Length")
                        from: pluginDB.getMinValue("overlapLength")
                        to: pluginDB.getMaxValue("overlapLength")
                        value: pluginDB.overlapLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.overlapLength = v;
                        }
                    }

                }

            }

            Kirigami.Card {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Pitch")
                    level: 2
                }

                contentItem: Column {
                    spacing: Kirigami.Units.gridUnit

                    EeSpinBox {
                        id: semitones

                        label: i18n("Semitones")
                        from: pluginDB.getMinValue("semitones")
                        to: pluginDB.getMaxValue("semitones")
                        value: pluginDB.semitones
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: (v) => {
                            pluginDB.semitones = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: tempoDifference

                        label: i18n("Tempo Difference")
                        from: pluginDB.getMinValue("tempoDifference")
                        to: pluginDB.getMaxValue("tempoDifference")
                        value: pluginDB.tempoDifference
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.tempoDifference = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: rateDifference

                        label: i18n("Rate Difference")
                        from: pluginDB.getMinValue("rateDifference")
                        to: pluginDB.getMaxValue("rateDifference")
                        value: pluginDB.rateDifference
                        decimals: 0
                        stepSize: 1
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.rateDifference = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: pitchPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.soundTouch}</b>`)
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
                    text: i18n("Quick Seek")
                    icon.name: "media-seek-forward-symbolic"
                    checkable: true
                    checked: pluginDB.quickSeek
                    onTriggered: {
                        if (pluginDB.quickSeek !== checked)
                            pluginDB.quickSeek = checked;

                    }
                },
                Kirigami.Action {
                    text: i18n("Anti-aliasing")
                    icon.name: "filter-symbolic"
                    checkable: true
                    checked: pluginDB.antiAlias
                    onTriggered: {
                        if (pluginDB.antiAlias !== checked)
                            pluginDB.antiAlias = checked;

                    }
                },
                Kirigami.Action {
                    text: i18n("Reset Settings")
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        pluginBackend.reset();
                    }
                }
            ]
        }

    }

}
