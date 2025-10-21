import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName// qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: pitchPage

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
            minimumColumnWidth: Kirigami.Units.gridUnit * 17
            uniformCellWidths: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Quality") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: sequenceLength

                        label: i18n("Sequence Length") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pitchPage.pluginDB.getMinValue("sequenceLength")
                        to: pitchPage.pluginDB.getMaxValue("sequenceLength")
                        value: pitchPage.pluginDB.sequenceLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            pitchPage.pluginDB.sequenceLength = v;
                        }
                    }

                    EeSpinBox {
                        id: seekWindow

                        label: i18n("Seek Window") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pitchPage.pluginDB.getMinValue("seekWindow")
                        to: pitchPage.pluginDB.getMaxValue("seekWindow")
                        value: pitchPage.pluginDB.seekWindow
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            pitchPage.pluginDB.seekWindow = v;
                        }
                    }

                    EeSpinBox {
                        id: overlapLength

                        label: i18n("Overlap Length") // qmllint disable
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 6
                        from: pitchPage.pluginDB.getMinValue("overlapLength")
                        to: pitchPage.pluginDB.getMaxValue("overlapLength")
                        value: pitchPage.pluginDB.overlapLength
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: v => {
                            pitchPage.pluginDB.overlapLength = v;
                        }
                    }
                }
            }

            Kirigami.Card {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Pitch") // qmllint disable
                    level: 2
                }

                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.gridUnit

                    GridLayout {
                        columns: 2
                        uniformCellWidths: true
                        Layout.alignment: Qt.AlignTop

                        EeSpinBox {
                            id: octaves

                            Layout.columnSpan: 2
                            label: i18n("Octaves") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pitchPage.pluginDB.getMinValue("octaves")
                            to: pitchPage.pluginDB.getMaxValue("octaves")
                            value: pitchPage.pluginDB.octaves
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pitchPage.pluginDB.octaves = v;
                            }
                        }

                        EeSpinBox {
                            id: cents

                            label: i18n("Cents") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pitchPage.pluginDB.getMinValue("cents")
                            to: pitchPage.pluginDB.getMaxValue("cents")
                            value: pitchPage.pluginDB.cents
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pitchPage.pluginDB.cents = v;
                            }
                        }

                        EeSpinBox {
                            id: semitones

                            label: i18n("Semitones") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pitchPage.pluginDB.getMinValue("semitones")
                            to: pitchPage.pluginDB.getMaxValue("semitones")
                            value: pitchPage.pluginDB.semitones
                            decimals: 0
                            stepSize: 1
                            onValueModified: v => {
                                pitchPage.pluginDB.semitones = v;
                            }
                        }

                        EeSpinBox {
                            id: tempoDifference

                            label: i18n("Tempo Difference") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pitchPage.pluginDB.getMinValue("tempoDifference")
                            to: pitchPage.pluginDB.getMaxValue("tempoDifference")
                            value: pitchPage.pluginDB.tempoDifference
                            decimals: 0
                            stepSize: 1
                            unit: "%"
                            onValueModified: v => {
                                pitchPage.pluginDB.tempoDifference = v;
                            }
                        }

                        EeSpinBox {
                            id: rateDifference

                            label: i18n("Rate Difference") // qmllint disable
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pitchPage.pluginDB.getMinValue("rateDifference")
                            to: pitchPage.pluginDB.getMaxValue("rateDifference")
                            value: pitchPage.pluginDB.rateDifference
                            decimals: 0
                            stepSize: 1
                            unit: "%"
                            onValueModified: v => {
                                pitchPage.pluginDB.rateDifference = v;
                            }
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.soundTouch}</strong>`) // qmllint disable
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
                        pitchPage.pluginBackend.resetHistory();
                    }
                },
                Kirigami.Action {
                    text: i18n("Quick Seek") // qmllint disable
                    icon.name: "media-seek-forward-symbolic"
                    checkable: true
                    checked: pitchPage.pluginDB.quickSeek
                    onTriggered: {
                        if (pitchPage.pluginDB.quickSeek !== checked)
                            pitchPage.pluginDB.quickSeek = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Anti-aliasing") // qmllint disable
                    icon.name: "filter-symbolic"
                    checkable: true
                    checked: pitchPage.pluginDB.antiAlias
                    onTriggered: {
                        if (pitchPage.pluginDB.antiAlias !== checked)
                            pitchPage.pluginDB.antiAlias = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset Settings") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        pitchPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
