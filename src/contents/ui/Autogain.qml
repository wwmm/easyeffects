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
        Kirigami.CardsLayout {
            maximumColumnWidth: {
                cardLevels.implicitWidth;
            }

            Kirigami.Card {
                id: cardControls

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

                header: Kirigami.Heading {
                    text: i18n("Loudness")
                    level: 2
                }

                contentItem: GridLayout {
                    Layout.fillWidth: true
                    columns: 3
                    columnSpacing: Kirigami.Units.smallSpacing

                    FormCard.FormTextDelegate {
                        text: i18n("Momentary")
                    }

                    Controls.ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: 50
                        indeterminate: false
                    }

                    FormCard.FormTextDelegate {
                        text: i18n("LUFS")
                    }

                    FormCard.FormTextDelegate {
                        text: i18n("Short-Term")
                    }

                    Controls.ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: 50
                        indeterminate: false
                    }

                    FormCard.FormTextDelegate {
                        text: i18n("LUFS")
                    }

                    FormCard.FormTextDelegate {
                        text: i18n("Integrated")
                    }

                    Controls.ProgressBar {
                        Layout.fillWidth: true
                        from: 0
                        to: 100
                        value: 50
                        indeterminate: false
                    }

                    FormCard.FormTextDelegate {
                        text: i18n("LUFS")
                    }

                }

            }

        }

        EeInputOutputGain {
            Layout.fillWidth: true
        }

    }

    footer: Controls.ToolBar {
        RowLayout {
            anchors.fill: parent

            Controls.Label {
                text: i18n("Using") + EEtagsPluginPackage.ebur128
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }

            Controls.ToolButton {
                text: i18n("Reset")
                onClicked: showPassiveNotification("Resetting the autogain")
            }

        }

    }

}
