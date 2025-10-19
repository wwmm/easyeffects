//pragma explanation: https://doc.qt.io/qt-6/qtqml-documents-structure.html
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: crystalizerPage

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

        const newData = pluginBackend.getAdaptiveIntensities();

        for (let n = 0; n < crystalizerPage.pluginBackend.numBands; n++) {
            listModel.setProperty(n, "adaptiveIntensity", newData[n]);
        }
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);

        for (let n = 0; n < crystalizerPage.pluginBackend.numBands; n++) {
            listModel.append({
                "adaptiveIntensity": 1.0
            });
        }
    }

    ListModel {
        id: listModel
    }

    ColumnLayout {
        height: crystalizerPage.height - crystalizerPage.header.height - crystalizerPage.footer.height - Kirigami.Units.gridUnit

        Kirigami.Card {
            id: cardControls

            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: false
            Layout.fillHeight: true

            footer: Controls.ScrollBar {
                id: listViewScrollBar

                Layout.fillWidth: true
            }

            contentItem: ListView {
                id: listview

                implicitWidth: contentItem.childrenRect.width < crystalizerPage.width ? contentItem.childrenRect.width : crystalizerPage.width - 4 * (cardControls.leftPadding + cardControls.rightPadding)
                clip: true
                reuseItems: true
                orientation: ListView.Horizontal
                // model: crystalizerPage.pluginBackend.numBands
                model: listModel
                Controls.ScrollBar.horizontal: listViewScrollBar

                delegate: CrystalizerBand {
                    pluginDB: crystalizerPage.pluginDB
                    pluginBackend: crystalizerPage.pluginBackend
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
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.zita}</strong>`) // qmllint disable
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
                    text: i18n("Adaptive Intensity")// qmllint disable
                    icon.name: "auto-scale-y"
                    checkable: true
                    checked: crystalizerPage.pluginDB.adaptiveIntensity
                    onTriggered: {
                        if (crystalizerPage.pluginDB.adaptiveIntensity != checked)
                            crystalizerPage.pluginDB.adaptiveIntensity = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        crystalizerPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
