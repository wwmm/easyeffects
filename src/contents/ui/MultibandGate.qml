import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: multibandGatePage

    required property var name
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
    // reductionLevel.value = pluginBackend.getReductionLevel();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            maximumColumns: 3
            FormCard.FormComboBoxDelegate {
                id: gateMode

                text: i18n("Operating Mode")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: pluginDB.gateMode
                editable: false
                model: [i18n("Classic"), i18n("Modern"), i18n("Linear Phase")]
                onActivated: idx => {
                    pluginDB.gateMode = idx;
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
                onValueModified: v => {
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
                onValueModified: v => {
                    pluginDB.wet = v;
                }
            }
        }
        RowLayout {
            ColumnLayout {
                Kirigami.CardsLayout {
                    Kirigami.Card {
                        id: bandCard

                        readonly property string bandId: "band" + bandsListview.currentIndex

                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop

                        header: Kirigami.Heading {
                            text: i18n("Band") + " " + (bandsListview.currentIndex + 1)
                            level: 2
                        }

                        contentItem: ColumnLayout {

                            Kirigami.ActionToolBar {
                                Layout.margins: Kirigami.Units.smallSpacing
                                alignment: Qt.AlignHCenter
                                position: Controls.ToolBar.Header
                                flat: true
                                actions: [
                                    Kirigami.Action {
                                        text: i18n("Mute")
                                        checkable: true
                                        checked: pluginDB[bandCard.bandId + "Mute"]
                                        onTriggered: {
                                            if (pluginDB[bandCard.bandId + "Mute"] != checked)
                                                pluginDB[bandCard.bandId + "Mute"] = checked;
                                        }
                                    },
                                    Kirigami.Action {
                                        text: i18n("Solo")
                                        checkable: true
                                        checked: pluginDB[bandCard.bandId + "Solo"]
                                        onTriggered: {
                                            if (pluginDB[bandCard.bandId + "Solo"] != checked)
                                                pluginDB[bandCard.bandId + "Solo"] = checked;
                                        }
                                    },
                                    Kirigami.Action {
                                        text: i18n("Bypass")
                                        checkable: true
                                        checked: !pluginDB[bandCard.bandId + "GateEnable"]
                                        onTriggered: {
                                            pluginDB[bandCard.bandId + "GateEnable"] = !checked;
                                        }
                                    }
                                ]
                            }
                        }
                    }
                }
            }

            Kirigami.Card {
                Layout.fillHeight: true
                Layout.fillWidth: false
                Layout.preferredHeight: contentItem.childrenRect.height + 2 * padding
                contentItem: ListView {
                    id: bandsListview

                    Layout.fillHeight: true
                    Layout.fillWidth: false
                    Layout.preferredHeight: contentItem.childrenRect.height
                    implicitHeight: contentItem.childrenRect.height

                    model: 8
                    implicitWidth: contentItem.childrenRect.width
                    clip: true
                    delegate: Controls.ItemDelegate {
                        id: listItemDelegate

                        required property int index

                        hoverEnabled: true
                        highlighted: ListView.isCurrentItem
                        onClicked: {
                            ListView.view.currentIndex = index;
                        }

                        contentItem: RowLayout {
                            Controls.Label {
                                Layout.fillWidth: true
                                text: i18n("Band") + " " + (listItemDelegate.index + 1)
                            }

                            Controls.CheckBox {
                                readonly property string bandName: "band" + listItemDelegate.index + "Enable"
                                Layout.alignment: Qt.AlignHCenter
                                visible: listItemDelegate.index > 0
                                checked: listItemDelegate.index > 0 ? pluginDB[bandName] : false
                                onCheckedChanged: {
                                    if (checked != pluginDB[bandName]) {
                                        pluginDB[bandName] = checked;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: multibandGatePage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.lsp}</b>`)
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
                    text: i18n("Stereo Split")
                    icon.name: "view-split-left-right-symbolic"
                    checkable: true
                    checked: pluginDB.stereoSplit
                    onTriggered: {
                        if (pluginDB.stereoSplit != checked)
                            pluginDB.stereoSplit = checked;
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
