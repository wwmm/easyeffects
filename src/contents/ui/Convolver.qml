import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: convolverPage

    required property var name
    required property var pluginDB
    required property var pipelineInstance
    property var pluginBackend: null

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
    }

    function validChartMag(chartMag) {
        // Determine if chartMag is a non-empty QList.
        return chartMag?.length > 0;
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);

        const chart = chartChannel.left ? pluginBackend.chartMagL : pluginBackend.chartMagR;
        if (validChartMag(chart)) {
            convolverChart.updateData(chart);
        }
    }

    Connections {
        function onKernelCombinationStopped() {
            progressBar.visible = false;
        }

        function onChartMagLChanged() {
            if (validChartMag(pluginBackend.chartMagL) && chartChannel.left && !spectrumAction.checked)
                convolverChart.updateData(pluginBackend.chartMagL);
        }

        function onChartMagRChanged() {
            if (validChartMag(pluginBackend.chartMagR) && chartChannel.right && !spectrumAction.checked) {
                convolverChart.updateData(pluginBackend.chartMagR);
            }
        }

        function onChartMagLfftLinearChanged() {
            if (validChartMag(pluginBackend.chartMagLfftLinear) && chartChannel.left && spectrumAction.checked && !convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(pluginBackend.chartMagLfftLinear);
        }

        function onChartMagRfftLinearChanged() {
            if (validChartMag(pluginBackend.chartMagRfftLinear) && chartChannel.right && spectrumAction.checked && !convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(pluginBackend.chartMagRfftLinear);
        }

        function onChartMagLfftLogChanged() {
            if (validChartMag(pluginBackend.chartMagLfftLog) && chartChannel.left && spectrumAction.checked && convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(pluginBackend.chartMagLfftLog);
        }

        function onChartMagRfftLogChanged() {
            if (validChartMag(pluginBackend.chartMagRfftLog) && chartChannel.right && spectrumAction.checked && convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(pluginBackend.chartMagRfftLog);
        }

        target: pluginBackend
    }

    ConvolverImpulseSheet {
        id: impulseSheet

        pluginDB: convolverPage.pluginDB
    }

    Kirigami.Dialog {
        id: combineDialog

        title: "Combine Impulse Responses"
        padding: Kirigami.Units.largeSpacing
        standardButtons: Kirigami.Dialog.NoButton
        flatFooterButtons: false

        ColumnLayout {
            FormCard.FormComboBoxDelegate {
                id: firstImpulse

                Layout.columnSpan: 2
                text: i18n("First")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: 0
                editable: false
                textRole: "name"
                model: Presets.SortedImpulseListModel
            }

            FormCard.FormComboBoxDelegate {
                id: secondImpulse

                Layout.columnSpan: 2
                text: i18n("Second")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: 0
                editable: false
                textRole: "name"
                model: Presets.SortedImpulseListModel
            }

            FormCard.FormTextFieldDelegate {
                id: combinedImpulseName

                label: "Output Impulse Name"
            }

            Controls.ProgressBar {
                id: progressBar

                Layout.fillWidth: true
                Layout.maximumWidth: firstImpulse.contentItem.implicitWidth
                Layout.alignment: Qt.AlignHCenter
                from: 0
                to: 100
                indeterminate: true
                visible: false
            }
        }

        customFooterActions: Kirigami.Action {
            text: i18n("Combine")
            icon.name: "path-combine-symbolic"
            onTriggered: {
                progressBar.visible = true;
                pluginBackend.combineKernels(firstImpulse.currentText, secondImpulse.currentText, combinedImpulseName.text);
                combinedImpulseName.clear();
            }
        }
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
                        impulseSheet.open();
                    }
                },
                Kirigami.Action {
                    text: qsTr("Combine")
                    icon.name: "path-combine-symbolic"
                    onTriggered: {
                        combineDialog.open();
                    }
                },
                Kirigami.Action {
                    id: spectrumAction

                    text: i18n("Spectrum")
                    icon.name: "folder-chart-symbolic"
                    checkable: true
                    onTriggered: {
                        if (checked) {
                            convolverChart.xUnit = "Hz";
                            if (!convolverChart.logarithimicHorizontalAxis) {
                                const chart = chartChannel.left ? pluginBackend.chartMagLfftLinear : pluginBackend.chartMagRfftLinear;

                                if (validChartMag(chart))
                                    convolverChart.updateData(chart);
                            } else {
                                const chart = chartChannel.left ? pluginBackend.chartMagLfftLog : pluginBackend.chartMagRfftLog;

                                if (validChartMag(chart))
                                    convolverChart.updateData(chart);
                            }
                        } else {
                            convolverChart.xUnit = "s";

                            const chart = chartChannel.left ? pluginBackend.chartMagL : pluginBackend.chartMagR;
                            if (validChartMag(chart))
                                convolverChart.updateData(chart);

                            spectrumLogScale.checked = false;
                            convolverChart.logarithimicHorizontalAxis = checked;
                        }
                    }
                },
                Kirigami.Action {
                    id: spectrumLogScale

                    text: qsTr("Log Scale")
                    visible: spectrumAction.checked
                    checkable: true
                    icon.name: "transform-scale-symbolic"
                    onTriggered: {
                        convolverChart.logarithimicHorizontalAxis = checked;
                        if (checked) {
                            const chart = chartChannel.left ? pluginBackend.chartMagLfftLog : pluginBackend.chartMagRfftLog;

                            if (validChartMag(chart))
                                convolverChart.updateData(chart);
                        } else {
                            const chart = chartChannel.left ? pluginBackend.chartMagLfftLinear : pluginBackend.chartMagRfftLinear;

                            if (validChartMag(chart))
                                convolverChart.updateData(chart);
                        }
                    }
                },
                Kirigami.Action {
                    id: chartChannel

                    property bool left: true
                    property bool right: false

                    displayComponent: RowLayout {
                        Controls.RadioButton {
                            id: radioLeft

                            text: i18n("Left")
                            checked: chartChannel.left
                            onCheckedChanged: {
                                if (checked !== chartChannel.left) {
                                    chartChannel.left = checked;
                                    if (!spectrumAction.checked) {
                                        if (validChartMag(pluginBackend.chartMagL))
                                            convolverChart.updateData(pluginBackend.chartMagL);
                                    } else {
                                        const chart = !convolverChart.logarithimicHorizontalAxis ? pluginBackend.chartMagLfftLinear : pluginBackend.chartMagLfftLog;
                                        if (validChartMag(chart))
                                            convolverChart.updateData(chart);
                                    }
                                }
                            }
                        }

                        Controls.RadioButton {
                            text: i18n("Right")
                            checked: chartChannel.right
                            onCheckedChanged: {
                                if (checked !== chartChannel.right) {
                                    chartChannel.right = checked;
                                    if (!spectrumAction.checked) {
                                        if (validChartMag(pluginBackend.chartMagR))
                                            convolverChart.updateData(pluginBackend.chartMagR);
                                    } else {
                                        const chart = !convolverChart.logarithimicHorizontalAxis ? pluginBackend.chartMagRfftLinear : pluginBackend.chartMagRfftLog;
                                        if (validChartMag(chart))
                                            convolverChart.updateData(chart);
                                    }
                                }
                            }
                        }
                    }
                }
            ]

            banner {
                title: pluginDB.kernelName
                titleAlignment: Qt.AlignHCenter | Qt.AlignBottom
                titleLevel: 2
                titleIcon: "waveform-symbolic"
            }

            contentItem: ColumnLayout {
                EeChart {
                    id: convolverChart

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    seriesType: 1 // spline series
                    colorScheme: DB.Manager.spectrum.spectrumColorScheme
                    colorTheme: DB.Manager.spectrum.spectrumColorTheme
                    xUnit: "s"
                    xAxisDecimals: 1
                    logarithimicHorizontalAxis: false
                    onWidthChanged: {
                        if (pluginBackend)
                            pluginBackend.interpPoints = convolverChart.width;
                    }
                }

                GridLayout {
                    id: delegateLayout

                    Layout.alignment: Qt.AlignHCenter
                    uniformCellWidths: true
                    rowSpacing: Kirigami.Units.largeSpacing
                    columnSpacing: Kirigami.Units.largeSpacing
                    columns: 3
                    rows: 2

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Rate")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Samples")
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Duration")
                    }

                    Controls.Label {
                        id: irRate

                        Layout.alignment: Qt.AlignHCenter
                        text: (pluginBackend ? pluginBackend.kernelRate : "") + " Hz"
                        enabled: false
                    }

                    Controls.Label {
                        id: irSamples

                        Layout.alignment: Qt.AlignHCenter
                        text: pluginBackend ? pluginBackend.kernelSamples : "0"
                        enabled: false
                    }

                    Controls.Label {
                        id: irDuration

                        Layout.alignment: Qt.AlignHCenter
                        text: Number(pluginBackend ? pluginBackend.kernelDuration : 0).toLocaleString(Qt.locale(), 'f', 3) + " s"
                        enabled: false
                    }
                }
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
