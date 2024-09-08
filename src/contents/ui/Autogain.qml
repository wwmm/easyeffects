import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ScrollablePage {
    id: autogainPage

    property var pluginDB

    anchors {
        left: parent.left
        top: parent.top
        right: parent.right
        bottom: parent.bottom
        leftMargin: Kirigami.Units.mediumSpacing
        rightMargin: Kirigami.Units.mediumSpacing
        topMargin: Kirigami.Units.mediumSpacing
        bottomMargin: Kirigami.Units.mediumSpacing
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            maximumColumnWidth: cardControls.maximumWidth

            FormCard.FormCardPage {
                FormCard.FormCard {
                    id: cardControls

                    FormCard.FormHeader {
                        title: i18n("Controls")
                    }

                    FormCard.FormComboBoxDelegate {
                        id: reference

                        text: i18n("Shape")
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

            FormCard.FormCardPage {
                FormCard.FormCard {
                    id: cardLevels

                    FormCard.FormHeader {
                        title: i18n("Loudness")
                    }

                    GridLayout {
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

        }

    }

    footer: RowLayout {
        Layout.fillWidth: true

        Controls.Label {
            Layout.alignment: Qt.AlignLeft
            font.bold: true
            text: i18n("Using") + EEtagsPluginPackage.ebur128
        }

        Controls.Button {
            Layout.alignment: Qt.AlignRight
            text: i18n("Reset")
            onClicked: showPassiveNotification("Reset")
        }

    }

}
