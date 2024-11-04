import "Common.js" as Common
import EEdbm
import EEpw
import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.ScrollablePage {
    id: limiterPage

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
        Layout.fillWidth: true
        Layout.fillHeight: true

        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true
            maximumColumns: 3

            Kirigami.Card {
                id: cardMode

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: mode

                        text: i18n("Mode")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.mode
                        editable: false
                        model: [i18n("Herm Thin"), i18n("Herm Wide"), i18n("Herm Tail"), i18n("Herm Duck"), i18n("Exp Thin"), i18n("Exp Wide"), i18n("Exp Tail"), i18n("Exp Duck"), i18n("Line Thin"), i18n("Line Wide"), i18n("Line Tail"), i18n("Line Duck")]
                        onActivated: (idx) => {
                            pluginDB.mode = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: oversampling

                        text: i18n("Oversampling")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.oversampling
                        editable: false
                        model: [i18n("None"), i18n("Half x2/16 bit"), i18n("Half x2/24 bit"), i18n("Half x3/16 bit"), i18n("Half x3/24 bit"), i18n("Half x4/16 bit"), i18n("Half x4/24 bit"), i18n("Half x6/16 bit"), i18n("Half x6/24 bit"), i18n("Half x8/16 bit"), i18n("Half x8/24 bit"), i18n("Full x2/16 bit"), i18n("Full x2/24 bit"), i18n("Full x3/16 bit"), i18n("Full x3/24 bit"), i18n("Full x4/16 bit"), i18n("Full x4/24 bit"), i18n("Full x6/16 bit"), i18n("Full x6/24 bit"), i18n("Full x8/16 bit"), i18n("Full x8/24 bit")]
                        onActivated: (idx) => {
                            pluginDB.oversampling = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: dithering

                        text: i18n("Dithering")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.dithering
                        editable: false
                        model: [i18n("None"), i18n("7 bit"), i18n("8 bit"), i18n("11 bit"), i18n("12 bit"), i18n("15 bit"), i18n("16 bit"), i18n("23 bit"), i18n("24 bit")]
                        onActivated: (idx) => {
                            pluginDB.dithering = idx;
                        }
                    }

                }

            }

            Kirigami.Card {
                id: cardLimiter

                header: Kirigami.Heading {
                    text: i18n("Limiter")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: threshold

                        label: i18n("Threshold")
                        from: -48
                        to: 0
                        value: pluginDB.threshold
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.threshold = v;
                        }
                    }

                    EeSpinBox {
                        id: attack

                        label: i18n("Attack")
                        from: 0.25
                        to: 20
                        value: pluginDB.attack
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.attack = v;
                        }
                    }

                    EeSpinBox {
                        id: release

                        label: i18n("Release")
                        from: 0.25
                        to: 20
                        value: pluginDB.release
                        decimals: 2
                        stepSize: 0.01
                        unit: "ms"
                        onValueModified: (v) => {
                            pluginDB.release = v;
                        }
                    }

                    EeSpinBox {
                        id: stereoLink

                        label: i18n("Stereo Link")
                        from: 0
                        to: 100
                        value: pluginDB.stereoLink
                        decimals: 1
                        stepSize: 0.1
                        unit: "%"
                        onValueModified: (v) => {
                            pluginDB.stereoLink = v;
                        }
                    }

                }

            }

            Kirigami.Card {
                id: cardSideChain

                header: Kirigami.Heading {
                    text: i18n("Sidechain")
                    level: 2
                }

                contentItem: ColumnLayout {
                    FormCard.FormComboBoxDelegate {
                        id: sidechainType

                        text: i18n("SC Type")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: pluginDB.sidechainType
                        editable: false
                        model: [i18n("Internal"), i18n("External"), i18n("Link")]
                        onActivated: (idx) => {
                            pluginDB.sidechainType = idx;
                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: comboSideChainInputDevice

                        text: i18n("Name")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        editable: false
                        model: ModelNodes
                        textRole: "description"
                        enabled: sidechainType.currentIndex === 1
                        currentIndex: {
                            for (let n = 0; n < ModelNodes.rowCount(); n++) {
                                if (ModelNodes.getNodeName(n) === pluginDB.sidechainInputDevice)
                                    return n;

                            }
                            return 0;
                        }
                        onActivated: (idx) => {
                            let selectedName = ModelNodes.getNodeName(idx);
                            if (selectedName !== pluginDB.sidechainInputDevice)
                                pluginDB.sidechainInputDevice = selectedName;

                        }
                    }

                    EeSpinBox {
                        id: sidechainPreamp

                        label: i18n("SC Preamp")
                        from: Common.minimumDecibelLevel
                        to: 40
                        value: pluginDB.sidechainPreamp
                        decimals: 2
                        stepSize: 0.01
                        unit: "dB"
                        onValueModified: (v) => {
                            pluginDB.sidechainPreamp = v;
                        }
                    }

                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: limiterPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.lsp
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
                    text: i18n("Gain Boost")
                    icon.name: "usermenu-up-symbolic"
                    checkable: true
                    checked: pluginDB.gainBoost
                    onTriggered: {
                        if (pluginDB.gainBoost != checked)
                            pluginDB.gainBoost = checked;

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
