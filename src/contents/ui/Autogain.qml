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

    RowLayout {
        spacing: Kirigami.Units.largeSpacing

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: Kirigami.Units.smallSpacing
        }

        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("Controls")
            }

            FormCard.FormCard {
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

            }

        }

        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("Loudness")
            }

            FormCard.FormCard {
                RowLayout {
                    Layout.columnSpan: appWindow.wideScreen ? 3 : 1
                    spacing: Kirigami.Units.smallSpacing

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

                }

                RowLayout {
                    Layout.columnSpan: appWindow.wideScreen ? 3 : 1
                    spacing: Kirigami.Units.smallSpacing

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

                }

                RowLayout {
                    Layout.columnSpan: appWindow.wideScreen ? 3 : 1
                    spacing: Kirigami.Units.smallSpacing

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

    header: Kirigami.ActionToolBar {
        alignment: Qt.AlignCenter
        actions: [
            Kirigami.Action {

                displayComponent: ColumnLayout {
                    Controls.Label {
                        text: i18n("Reference")
                    }

                    Controls.ComboBox {
                        id: reference

                        Layout.fillWidth: true
                        // currentIndex: EEdbSpectrum.spectrumShape
                        editable: false
                        model: [i18n("Momentary"), i18n("Short-Term"), i18n("Integrated"), i18n("Geometric Mean (MSI)"), i18n("Geometric Mean (MS)"), i18n("Geometric Mean (MI)"), i18n("Geometric Mean (SI)")]
                        onActivated: (idx) => {
                        }
                    }

                }

            },
            Kirigami.Action {

                displayComponent: EeSpinBox {
                    id: target2

                    label: i18n("Target")
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: -100
                    // value: pluginDB.target
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    onValueModified: (v) => {
                        pluginDB.target = v;
                    }
                }

            },
            Kirigami.Action {

                displayComponent: EeSpinBox {
                    id: silenceThreshold

                    label: i18n("Silence")
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: -100
                    to: 0
                    // value: pluginDB.target
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    onValueModified: (v) => {
                        pluginDB.target = v;
                    }
                }

            }
        ]

        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }

    }

    footer: RowLayout {
        Layout.fillWidth: true

        Controls.Button {
            Layout.alignment: Qt.AlignCenter
            text: i18n("Reset")
            onClicked: showPassiveNotification("Reset")
        }

        Controls.Label {
            Layout.alignment: Qt.AlignRight
            font.bold: true
            text: i18n("Using") + EEtagsPluginPackage.ebur128
        }

    }

}
