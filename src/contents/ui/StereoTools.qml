import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.tags.plugin.name as TagsPluginName
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

            maximumColumns: 4
            uniformCellWidths: true

            Kirigami.Card {

                header: Kirigami.Heading {
                    text: i18n("Input")
                    level: 2
                }

                contentItem: Column {
                    EeSwitch {
                        id: softclip

                        label: i18n("Softclip")
                        isChecked: pluginDB.softclip
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.softclip)
                                pluginDB.softclip = isChecked;

                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

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
                        enabled: pluginDB.softclip
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

                        EeSpinBox {
                            id: slev

                            label: i18n("Side Level")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("slev")
                            to: pluginDB.getMaxValue("slev")
                            value: pluginDB.slev
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
                                pluginDB.slev = v;
                            }
                        }

                        EeSpinBox {
                            id: sbal

                            label: i18n("Side Balance")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("sbal")
                            to: pluginDB.getMaxValue("sbal")
                            value: pluginDB.sbal
                            decimals: 2
                            stepSize: 0.01
                            onValueModified: (v) => {
                                pluginDB.sbal = v;
                            }
                        }

                        EeSpinBox {
                            id: mlev

                            label: i18n("Middle Level")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("mlev")
                            to: pluginDB.getMaxValue("mlev")
                            value: pluginDB.mlev
                            decimals: 2
                            stepSize: 0.01
                            unit: "dB"
                            minusInfinityMode: true
                            onValueModified: (v) => {
                                pluginDB.mlev = v;
                            }
                        }

                        EeSpinBox {
                            id: mpan

                            label: i18n("Middle Panorama")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("mpan")
                            to: pluginDB.getMaxValue("mpan")
                            value: pluginDB.mpan
                            decimals: 2
                            stepSize: 0.01
                            onValueModified: (v) => {
                                pluginDB.mpan = v;
                            }
                        }

                    }

                }

            }

            Kirigami.Card {

                contentItem: Column {
                    Kirigami.Heading {
                        text: i18n("Left")
                        level: 2

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSwitch {
                        id: mutel

                        label: i18n("Mute")
                        isChecked: pluginDB.mutel
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.mutel)
                                pluginDB.mutel = isChecked;

                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSwitch {
                        id: phasel

                        label: i18n("Invert Phase")
                        isChecked: pluginDB.phasel
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.phasel)
                                pluginDB.phasel = isChecked;

                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    Kirigami.Heading {
                        text: i18n("Right")
                        level: 2
                        topPadding: Kirigami.Units.gridUnit * 2

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSwitch {
                        id: muter

                        label: i18n("Mute")
                        isChecked: pluginDB.muter
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.muter)
                                pluginDB.muter = isChecked;

                        }

                        anchors {
                            left: parent.left
                            right: parent.right
                        }

                    }

                    EeSwitch {
                        id: phaser

                        label: i18n("Invert Phase")
                        isChecked: pluginDB.phaser
                        onCheckedChanged: {
                            if (isChecked !== pluginDB.phaser)
                                pluginDB.phaser = isChecked;

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
                    text: i18n("Output")
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
                        }

                        EeSpinBox {
                            id: delay

                            label: i18n("Delay")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("delay")
                            to: pluginDB.getMaxValue("delay")
                            value: pluginDB.delay
                            decimals: 2
                            stepSize: 0.01
                            unit: "ms"
                            onValueModified: (v) => {
                                pluginDB.delay = v;
                            }
                        }

                        EeSpinBox {
                            id: stereoBase

                            label: i18n("Stereo Base")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("stereoBase")
                            to: pluginDB.getMaxValue("stereoBase")
                            value: pluginDB.stereoBase
                            decimals: 2
                            stepSize: 0.01
                            onValueModified: (v) => {
                                pluginDB.stereoBase = v;
                            }
                        }

                        EeSpinBox {
                            id: stereoPhase

                            label: i18n("Stereo Phase")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: pluginDB.getMinValue("stereoPhase")
                            to: pluginDB.getMaxValue("stereoPhase")
                            value: pluginDB.stereoPhase
                            decimals: 0
                            stepSize: 1
                            unit: "°"
                            onValueModified: (v) => {
                                pluginDB.stereoPhase = v;
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
            text: i18n("Using %1", `<b>${TagsPluginName.Package.calf}</b>`)
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
                    text: i18n("Show Native Window")
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.showNativeUi();
                        else
                            pluginBackend.closeNativeUi();
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
