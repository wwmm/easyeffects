import "Common.js" as Common
import EEdbm
import EEtagsPluginName
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: crystalizerPage

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

    horizontalScrollBarPolicy: Qt.ScrollBarAsNeeded
    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    ColumnLayout {
        height: crystalizerPage.height - crystalizerPage.header.height - crystalizerPage.footer.height - Kirigami.Units.gridUnit

        Kirigami.Card {
            id: cardControls

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: false
            Layout.fillHeight: true
            implicitWidth: 53 * Kirigami.Units.gridUnit

            contentItem: ListView {
                id: listview

                clip: true
                reuseItems: false
                orientation: ListView.Horizontal
                model: 13

                delegate: CrystalizerBand {
                    pluginDB: {
                        crystalizerPage.pluginDB;
                    }
                }

            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: crystalizerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using") + EEtagsPluginPackage.zita
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
