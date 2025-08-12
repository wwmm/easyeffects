import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

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

    RowLayout {
        Kirigami.Card {
            id: cardControls

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            header: Kirigami.Heading {
                text: i18n("Controls")
                level: 2
            }

            contentItem: ColumnLayout {}
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
