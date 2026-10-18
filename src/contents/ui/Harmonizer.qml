pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: harmonizerPage

    required property string name
    required property DbHarmonizer pluginDB
    required property EffectsBase pipelineInstance
    property BackendHarmonizer pluginBackend

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
            Layout.fillWidth: true
            minimumColumnWidth: Kirigami.Units.gridUnit * 20
            maximumColumnWidth: Kirigami.Units.gridUnit * 23
            uniformCellWidths: true

            EeCard {
                title: i18n("Harmony") // qmllint disable

                Controls.ComboBox {
                    Layout.fillWidth: true
                    textRole: "text"
                    model: [
                        {
                            "text": i18n("C")
                        },
                        {
                            "text": i18n("C♯ / D♭")
                        },
                        {
                            "text": i18n("D")
                        },
                        {
                            "text": i18n("D♯ / E♭")
                        },
                        {
                            "text": i18n("E")
                        },
                        {
                            "text": i18n("F")
                        },
                        {
                            "text": i18n("F♯ / G♭")
                        },
                        {
                            "text": i18n("G")
                        },
                        {
                            "text": i18n("G♯ / A♭")
                        },
                        {
                            "text": i18n("A")
                        },
                        {
                            "text": i18n("A♯ / B♭")
                        },
                        {
                            "text": i18n("B")
                        }
                    ]
                    currentIndex: harmonizerPage.pluginDB.note
                    onActivated: index => harmonizerPage.pluginDB.note = index
                }

                Controls.ComboBox {
                    Layout.fillWidth: true
                    textRole: "text"
                    model: [
                        {
                            "text": i18n("No chord")
                        },
                        {
                            "text": i18n("6")
                        },
                        {
                            "text": i18n("Major 7")
                        },
                        {
                            "text": i18n("Lydian")
                        },
                        {
                            "text": i18n("Major 9")
                        },
                        {
                            "text": i18n("Major 7/9")
                        },
                        {
                            "text": i18n("6/9")
                        },
                        {
                            "text": i18n("Augmented")
                        },
                        {
                            "text": i18n("Minor")
                        },
                        {
                            "text": i18n("Minor 6")
                        },
                        {
                            "text": i18n("Minor 7")
                        },
                        {
                            "text": i18n("Minor 7 flat 5")
                        },
                        {
                            "text": i18n("Minor 9")
                        },
                        {
                            "text": i18n("Minor 7/9")
                        },
                        {
                            "text": i18n("Minor 7/11")
                        },
                        {
                            "text": i18n("Minor major 7")
                        },
                        {
                            "text": i18n("Minor major 7/9")
                        },
                        {
                            "text": i18n("Diminished")
                        },
                        {
                            "text": i18n("Diminished 7")
                        },
                        {
                            "text": i18n("Dominant 7")
                        },
                        {
                            "text": i18n("Dominant 7 sus4")
                        },
                        {
                            "text": i18n("Dominant 7 flat 5")
                        },
                        {
                            "text": i18n("Dominant 9")
                        },
                        {
                            "text": i18n("Dominant 7 sharp 11")
                        },
                        {
                            "text": i18n("Dominant 13")
                        },
                        {
                            "text": i18n("Dominant 7 flat 9")
                        },
                        {
                            "text": i18n("Dominant 7 flat 13")
                        },
                        {
                            "text": i18n("Dominant 7 sharp 9")
                        },
                        {
                            "text": i18n("Augmented major 7")
                        },
                        {
                            "text": i18n("Augmented 7")
                        },
                        {
                            "text": i18n("Octaves")
                        },
                        {
                            "text": i18n("Fifths")
                        },
                        {
                            "text": i18n("Suspended 4")
                        },
                        {
                            "text": i18n("Seconds")
                        }
                    ]
                    currentIndex: harmonizerPage.pluginDB.chord
                    onActivated: index => harmonizerPage.pluginDB.chord = index
                }

                EeSwitch {
                    label: i18n("Follow chord")
                    checked: harmonizerPage.pluginDB.chordMode
                    onCheckedChanged: harmonizerPage.pluginDB.chordMode = checked
                }

                EeSpinBox {
                    label: i18n("Left voice interval")
                    from: harmonizerPage.pluginDB.getMinValue("intervalLeft")
                    to: harmonizerPage.pluginDB.getMaxValue("intervalLeft")
                    value: harmonizerPage.pluginDB.intervalLeft
                    decimals: 0
                    stepSize: 1
                    onValueModified: value => harmonizerPage.pluginDB.intervalLeft = value
                }

                EeSpinBox {
                    label: i18n("Left voice level")
                    from: harmonizerPage.pluginDB.getMinValue("gainLeft")
                    to: harmonizerPage.pluginDB.getMaxValue("gainLeft")
                    value: harmonizerPage.pluginDB.gainLeft
                    decimals: 0
                    stepSize: 1
                    unit: Units.dB
                    onValueModified: value => harmonizerPage.pluginDB.gainLeft = value
                }

                EeSpinBox {
                    label: i18n("Right voice interval")
                    from: harmonizerPage.pluginDB.getMinValue("intervalRight")
                    to: harmonizerPage.pluginDB.getMaxValue("intervalRight")
                    value: harmonizerPage.pluginDB.intervalRight
                    decimals: 0
                    stepSize: 1
                    onValueModified: value => harmonizerPage.pluginDB.intervalRight = value
                }

                EeSpinBox {
                    label: i18n("Right voice level")
                    from: harmonizerPage.pluginDB.getMinValue("gainRight")
                    to: harmonizerPage.pluginDB.getMaxValue("gainRight")
                    value: harmonizerPage.pluginDB.gainRight
                    decimals: 0
                    stepSize: 1
                    unit: Units.dB
                    onValueModified: value => harmonizerPage.pluginDB.gainRight = value
                }

                EeSpinBox {
                    label: i18n("Dry/wet")
                    from: harmonizerPage.pluginDB.getMinValue("dryWet")
                    to: harmonizerPage.pluginDB.getMaxValue("dryWet")
                    value: harmonizerPage.pluginDB.dryWet
                    decimals: 0
                    stepSize: 1
                    onValueModified: value => harmonizerPage.pluginDB.dryWet = value
                }
            }
        }
    }

    EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: harmonizerPage.pluginDB
    }

    header: inputOutputLevels

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${PluginsPackage.rakarrack}</strong>`) // qmllint disable
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
                        harmonizerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
