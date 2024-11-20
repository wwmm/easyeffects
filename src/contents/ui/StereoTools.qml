import "Common.js" as Common
import EEdbm
import EEpw
import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: stereoToolsPage

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

            maximumColumns: 5
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Input")
                    level: 2
                }

                contentItem: Column {
                    EeSpinBox {
                        id: balanceIn

                        label: i18n("Balance")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("balanceIn")
                        to: pluginDB.getMaxValue("balanceIn")
                        value: pluginDB.balanceIn
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: (v) => {
                            pluginDB.balanceIn = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: scLevel

                        label: i18n("S/C Level")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("scLevel")
                        to: pluginDB.getMaxValue("scLevel")
                        value: pluginDB.scLevel
                        decimals: 3
                        stepSize: 0.001
                        onValueModified: (v) => {
                            pluginDB.scLevel = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                }

            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Stereo Matrix")
                    level: 2
                }

                contentItem: Column {
                    GridLayout {
                        columns: 2
                        uniformCellWidths: true

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                        FormCard.FormComboBoxDelegate {
                            id: mode

                            Layout.columnSpan: 2
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: pluginDB.mode
                            editable: false
                            model: [i18n("LR > LR (Stereo Default)"), i18n("LR > MS (Stereo to Mid-Side)"), i18n("MS > LR (Mid-Side to Stereo)"), i18n("LR > LL (Mono Left Channel)"), i18n("LR > RR (Mono Right Channel)"), i18n("LR > L+R (Mono Sum L+R)"), i18n("LR > RL (Stereo Flip Channels)")]
                            onActivated: (idx) => {
                                pluginDB.mode = idx;
                            }
                        }

                    }

                }

            }

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Output")
                    level: 2
                }

                contentItem: Column {
                    EeSpinBox {
                        id: balanceOut

                        label: i18n("Balance")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("balanceOut")
                        to: pluginDB.getMaxValue("balanceOut")
                        value: pluginDB.balanceOut
                        decimals: 2
                        stepSize: 0.01
                        onValueModified: (v) => {
                            pluginDB.balanceOut = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: dry

                        label: i18n("Dry")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("dry")
                        to: pluginDB.getMaxValue("dry")
                        value: pluginDB.dry
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: (v) => {
                            pluginDB.dry = v;
                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSpinBox {
                        id: wet

                        label: i18n("Wet")
                        labelAbove: true
                        spinboxLayoutFillWidth: true
                        from: pluginDB.getMinValue("wet")
                        to: pluginDB.getMaxValue("wet")
                        value: pluginDB.wet
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        minusInfinityMode: true
                        onValueModified: (v) => {
                            pluginDB.wet = v;
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

        pluginDB: stereoToolsPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.cal
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
                    text: i18n("Show Native Window")
                    icon.name: "window-duplicate-symbolic"
                    enabled: EEdbm.main.showNativePluginUi
                    checkable: true
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.show_native_ui();
                        else
                            pluginBackend.close_native_ui();
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
