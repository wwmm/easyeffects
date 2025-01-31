import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: convolverPage

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
        anchors {
            top: parent.top
            bottom: parent.bottom
        }

        Kirigami.Card {
            Layout.fillHeight: true
            actions: [
                Kirigami.Action {
                    text: qsTr("Impulses")
                    icon.name: "waveform-symbolic"
                    onTriggered: {
                    }
                },
                Kirigami.Action {
                    text: qsTr("Combine")
                    icon.name: "path-combine-symbolic"
                    onTriggered: {
                    }
                },
                Kirigami.Action {
                    id: spectrumAction

                    text: i18n("Spectrum")
                    icon.name: "folder-chart-symbolic"
                    checkable: true
                    onTriggered: {
                        if (checked)
                            convolverChart.xUnit = "Hz";
                        else
                            convolverChart.xUnit = "s";
                    }
                },
                Kirigami.Action {
                    text: qsTr("Log Scale")
                    visible: spectrumAction.checked
                    checkable: true
                    icon.name: "transform-scale-symbolic"
                    onTriggered: {
                        convolverChart.logarithimicHorizontalAxis = checked;
                    }
                }
            ]

            banner {
                title: "Impulse Name"
                titleAlignment: Qt.AlignHCenter | Qt.AlignBottom
            }

            contentItem: EeChart {
                id: convolverChart

                Layout.fillWidth: true
                Layout.fillHeight: true
                seriesType: 1 // spline series
                colorScheme: DB.Manager.spectrum.spectrumColorScheme
                colorTheme: DB.Manager.spectrum.spectrumColorTheme
                xUnit: "s"
                xMin: 0
                xMax: 10
                yMin: -100
                yMax: 0
                logarithimicHorizontalAxis: false
            }

        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: 0.5 * parent.width

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Stereo Width")
            }

            Controls.Slider {
                id: irWidth

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                value: pluginDB.irWidth
                from: pluginDB.getMinValue("irWidth")
                to: pluginDB.getMaxValue("irWidth")
                stepSize: 1
                onValueChanged: () => {
                    if (value !== pluginDB.irWidth)
                        pluginDB.irWidth = value;

                }
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: irWidth.value + " %"
            }

        }

    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: convolverPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.zita}</b>`)
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
                    text: i18n("Autogain")
                    icon.name: "audio-volume-medium-symbolic"
                    checkable: true
                    checked: pluginDB.autogain
                    onTriggered: {
                        if (checked !== pluginDB.autogain)
                            pluginDB.autogain = checked;

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
