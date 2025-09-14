import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: crossfeedPage

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
    }

    function applyPreset(name) {
        switch (name) {
        case "cmoy":
            pluginDB.fcut = 700;
            pluginDB.feed = 6;
            break;
        case "jmeier":
            pluginDB.fcut = 650;
            pluginDB.feed = 9.5;
            break;
        default:
            pluginDB.fcut = 700;
            pluginDB.feed = 4.5;
        }
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        Kirigami.CardsLayout {
            id: cardLayout

            Layout.fillWidth: true

            Kirigami.Card {
                id: cardControls

                header: Kirigami.Heading {
                    text: i18n("Controls")
                    level: 2
                }

                contentItem: ColumnLayout {
                    EeSpinBox {
                        id: fcut

                        label: i18n("Cutoff")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("fcut")
                        to: pluginDB.getMaxValue("fcut")
                        value: pluginDB.fcut
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        onValueModified: v => {
                            pluginDB.fcut = v;
                        }
                    }

                    EeSpinBox {
                        id: feed

                        label: i18n("Feed")
                        spinboxMaximumWidth: Kirigami.Units.gridUnit * 7
                        from: pluginDB.getMinValue("feed")
                        to: pluginDB.getMaxValue("feed")
                        value: pluginDB.feed
                        decimals: 1
                        stepSize: 0.1
                        unit: "dB"
                        onValueModified: v => {
                            pluginDB.feed = v;
                        }
                    }
                }
            }
        }
    }

    Kirigami.MenuDialog {
        id: presetsDialog

        title: i18n("Crossfeed Presets")
        actions: [
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: i18n("Default")
                onTriggered: {
                    applyPreset("default");
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: "Cmoy"
                onTriggered: {
                    applyPreset("cmoy");
                }
            },
            Kirigami.Action {
                icon.name: "bookmarks-symbolic"
                text: "Jmeier"
                onTriggered: {
                    applyPreset("jmeier");
                }
            }
        ]
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crossfeedPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.bs2b}</b>`)
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
                    text: i18n("Presets")
                    icon.name: "bookmarks-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    onTriggered: {
                        presetsDialog.open();
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
