import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ScrollablePage {
    id: autogainPage

    property var pluginDB

    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            maximumColumnWidth: Kirigami.Units.gridUnit * 20

            Kirigami.Card {
                id: cardControls

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: reference

                        text: i18n("Reference")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        // currentIndex: EEdbSpectrum.spectrumShape
                        editable: false
                        model: [i18n("Momentary"), i18n("Short-Term"), i18n("Integrated"), i18n("Geometric Mean (MSI)"), i18n("Geometric Mean (MS)"), i18n("Geometric Mean (MI)"), i18n("Geometric Mean (SI)")]
                        onActivated: (idx) => {
                        }
                    }

                    EeSpinBox {
                        id: target

                        label: i18n("Target")
                        from: -100
                        // value: pluginDB.target
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.target = v;
                        }
                    }

                    EeSpinBox {
                        id: silenceThreshold

                        label: i18n("Silence")
                        from: -100
                        to: 0
                        // value: pluginDB.silenceThreshold
                        decimals: 0
                        stepSize: 1
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.silenceThreshold = v;
                        }
                    }

                    EeSpinBox {
                        id: maximumHistory

                        label: i18n("Maximum History")
                        from: 6
                        to: 3600
                        // value: pluginDB.maximumHistory
                        decimals: 0
                        stepSize: 1
                        unit: "s"
                        onValueModified: (v) => {
                            pluginDB.maximumHistory = v;
                        }
                    }

                }

            }

            Kirigami.Card {
                id: cardLevels

                implicitWidth: cardLayout.maximumColumnWidth

                header: Kirigami.Heading {
                    text: i18n("Loudness")
                    level: 2
                }

                contentItem: ColumnLayout {
                    anchors {
                        left: parent.left
                        right: parent.right
                        top: parent.top
                    }

                    EeProgressBar {
                        label: i18n("Momentary")
                        unit: i18n("LUFS")
                        value: 0.5
                    }

                    EeProgressBar {
                        label: i18n("Short-Term")
                        unit: i18n("LUFS")
                        value: 0.4
                    }

                    EeProgressBar {
                        label: i18n("Integrated")
                        unit: i18n("LUFS")
                        value: 0.8
                    }

                    EeProgressBar {
                        label: i18n("Relative")
                        unit: i18n("LUFS")
                        value: 0.2
                    }

                    EeProgressBar {
                        label: i18n("Range")
                        unit: i18n("LU")
                        value: 0.7
                    }

                    EeProgressBar {
                        label: i18n("Loudness")
                        unit: i18n("LUFS")
                        value: 0.6
                    }

                    EeProgressBar {
                        label: i18n("Output Gain")
                        unit: i18n("dB")
                        value: 0.3
                    }

                }

            }

        }

    }

    header: Kirigami.ActionToolBar {
        Layout.margins: Kirigami.Units.smallSpacing
        alignment: Qt.AlignHCenter
        position: Controls.ToolBar.Header
        flat: true
        actions: [
            Kirigami.Action {

                displayComponent: EeSpinBox {
                    id: inputGain

                    label: i18n("Input")
                    from: -36
                    to: 36
                    // value: pluginDB.inputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        pluginDB.inputGain = v;
                    }
                }

            },
            Kirigami.Action {

                displayComponent: EeCircularProgress {
                    value: 0.6
                }

            },
            Kirigami.Action {

                displayComponent: EeCircularProgress {
                    value: 0.4
                }

            },
            Kirigami.Action {
                enabled: false
                separator: true
            },
            Kirigami.Action {

                displayComponent: EeSpinBox {
                    id: outputGain

                    label: i18n("Output")
                    from: -36
                    to: 36
                    // value: pluginDB.outputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        pluginDB.outputGain = v;
                    }
                }

            },
            Kirigami.Action {

                displayComponent: EeCircularProgress {
                    value: 0.3
                }

            },
            Kirigami.Action {

                displayComponent: EeCircularProgress {
                    value: 0.8
                }

            }
        ]
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.ebur128
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
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
                        showPassiveNotification("Resetting the autogain");
                    }
                }
            ]
        }

    }

}
