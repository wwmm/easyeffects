pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import ee.tags.plugin.name as TagsPluginName // qmllint disable
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: convolverPage

    required property string name
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
        function onNewKernelLoaded(name, success) {
            if (success) {
                convolverChartContainer.banner.title = name;

                appWindow.showStatus(i18n("Loaded the %1 Convolver impulse.", `<strong>${name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
            } else {
                convolverChartContainer.banner.title = i18n("Convolver impulse is not set");

                if (name.length === 0 || name === '""') {
                    // This is likely to happen after a config reset.
                    appWindow.showStatus(i18n("The Convolver is in passthrough mode.")); // qmllint disable
                } else {
                    appWindow.showStatus(i18n("Failed to load the %1 impulse. The Convolver is in passthrough mode.", `<strong>${name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                }

                convolverChart.clearData();
            }
        }

        function onKernelCombinationStopped() {
            progressBar.visible = false;
        }

        function onChartMagLChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagL) && chartChannel.left && !spectrumAction.checked)
                convolverChart.updateData(convolverPage.pluginBackend.chartMagL);
        }

        function onChartMagRChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagR) && chartChannel.right && !spectrumAction.checked) {
                convolverChart.updateData(convolverPage.pluginBackend.chartMagR);
            }
        }

        function onChartMagLfftLinearChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagLfftLinear) && chartChannel.left && spectrumAction.checked && !convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(convolverPage.pluginBackend.chartMagLfftLinear);
        }

        function onChartMagRfftLinearChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagRfftLinear) && chartChannel.right && spectrumAction.checked && !convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(convolverPage.pluginBackend.chartMagRfftLinear);
        }

        function onChartMagLfftLogChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagLfftLog) && chartChannel.left && spectrumAction.checked && convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(convolverPage.pluginBackend.chartMagLfftLog);
        }

        function onChartMagRfftLogChanged() {
            if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagRfftLog) && chartChannel.right && spectrumAction.checked && convolverChart.logarithimicHorizontalAxis)
                convolverChart.updateData(convolverPage.pluginBackend.chartMagRfftLog);
        }

        target: convolverPage.pluginBackend
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
                text: i18n("First") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: 0
                editable: false
                textRole: "name"
                model: Presets.SortedImpulseListModel
            }

            FormCard.FormComboBoxDelegate {
                id: secondImpulse

                Layout.columnSpan: 2
                text: i18n("Second") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: 0
                editable: false
                textRole: "name"
                model: Presets.SortedImpulseListModel
            }

            FormCard.FormTextFieldDelegate {
                id: combinedImpulseName

                label: "Output Impulse Name"

                validator: Validators.validFileNameRegex
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
            text: i18n("Combine") // qmllint disable
            icon.name: "path-combine-symbolic"
            onTriggered: {
                progressBar.visible = true;

                const saneCombinedImpulseName = combinedImpulseName.text.trim().replace(/(?:\.irs)+$/, "");

                convolverPage.pluginBackend.combineKernels(firstImpulse.currentText, secondImpulse.currentText, saneCombinedImpulseName);

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
            id: convolverChartContainer

            Layout.minimumWidth: Kirigami.Units.gridUnit * 16
            Layout.fillHeight: true

            actions: [
                Kirigami.Action {
                    text: i18n("Impulses")
                    icon.name: "waveform-symbolic"
                    onTriggered: {
                        impulseSheet.open();
                    }
                },
                Kirigami.Action {
                    text: i18n("Combine")
                    icon.name: "path-combine-symbolic"
                    onTriggered: {
                        combineDialog.open();
                    }
                },
                Kirigami.Action {
                    id: spectrumAction

                    text: i18n("Spectrum") // qmllint disable
                    icon.name: "folder-chart-symbolic"
                    checkable: true
                    onTriggered: {
                        if (checked) {
                            convolverChart.xUnit = i18n("Hz");
                            if (!convolverChart.logarithimicHorizontalAxis) {
                                const chart = chartChannel.left ? convolverPage.pluginBackend.chartMagLfftLinear : convolverPage.pluginBackend.chartMagRfftLinear;

                                if (convolverPage.validChartMag(chart))
                                    convolverChart.updateData(chart);
                            } else {
                                const chart = chartChannel.left ? convolverPage.pluginBackend.chartMagLfftLog : convolverPage.pluginBackend.chartMagRfftLog;

                                if (convolverPage.validChartMag(chart))
                                    convolverChart.updateData(chart);
                            }
                        } else {
                            convolverChart.xUnit = i18n("s");

                            const chart = chartChannel.left ? convolverPage.pluginBackend.chartMagL : convolverPage.pluginBackend.chartMagR;
                            if (convolverPage.validChartMag(chart))
                                convolverChart.updateData(chart);

                            spectrumLogScale.checked = false;
                            convolverChart.logarithimicHorizontalAxis = checked;
                        }
                    }
                },
                Kirigami.Action {
                    id: spectrumLogScale

                    text: i18n("Log scale")
                    visible: spectrumAction.checked
                    checkable: true
                    icon.name: "transform-scale-symbolic"
                    onTriggered: {
                        convolverChart.logarithimicHorizontalAxis = checked;
                        if (checked) {
                            const chart = chartChannel.left ? convolverPage.pluginBackend.chartMagLfftLog : convolverPage.pluginBackend.chartMagRfftLog;

                            if (convolverPage.validChartMag(chart))
                                convolverChart.updateData(chart);
                        } else {
                            const chart = chartChannel.left ? convolverPage.pluginBackend.chartMagLfftLinear : convolverPage.pluginBackend.chartMagRfftLinear;

                            if (convolverPage.validChartMag(chart))
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

                            text: i18n("Left") // qmllint disable
                            checked: chartChannel.left
                            onCheckedChanged: {
                                if (checked !== chartChannel.left) {
                                    chartChannel.left = checked;
                                    if (!spectrumAction.checked) {
                                        if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagL))
                                            convolverChart.updateData(convolverPage.pluginBackend.chartMagL);
                                    } else {
                                        const chart = !convolverChart.logarithimicHorizontalAxis ? convolverPage.pluginBackend.chartMagLfftLinear : convolverPage.pluginBackend.chartMagLfftLog;
                                        if (convolverPage.validChartMag(chart))
                                            convolverChart.updateData(chart);
                                    }
                                }
                            }
                        }

                        Controls.RadioButton {
                            text: i18n("Right") // qmllint disable
                            checked: chartChannel.right
                            onCheckedChanged: {
                                if (checked !== chartChannel.right) {
                                    chartChannel.right = checked;
                                    if (!spectrumAction.checked) {
                                        if (convolverPage.validChartMag(convolverPage.pluginBackend.chartMagR))
                                            convolverChart.updateData(convolverPage.pluginBackend.chartMagR);
                                    } else {
                                        const chart = !convolverChart.logarithimicHorizontalAxis ? convolverPage.pluginBackend.chartMagRfftLinear : convolverPage.pluginBackend.chartMagRfftLog;
                                        if (convolverPage.validChartMag(chart))
                                            convolverChart.updateData(chart);
                                    }
                                }
                            }
                        }
                    }
                }
            ]

            banner {
                title: {
                    const name = convolverPage.pluginDB.kernelName;
                    return (!convolverPage.pluginBackend?.kernelIsInitialized || name.length === 0 || name === '""') ? i18n("Convolver impulse is not set") : name; // qmllint disable
                }
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
                    xUnit: i18n("s")
                    xAxisDecimals: 2
                    logarithimicHorizontalAxis: false
                    onWidthChanged: {
                        if (convolverPage.pluginBackend)
                            convolverPage.pluginBackend.interpPoints = convolverChart.width;
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
                        text: i18n("Rate") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Samples") // qmllint disable
                    }

                    Controls.Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Duration") // qmllint disable
                    }

                    Controls.Label {
                        id: irRate

                        Layout.alignment: Qt.AlignHCenter
                        text: (convolverPage.pluginBackend ? convolverPage.pluginBackend.kernelRate : "") + ` ${i18n("Hz")}` // qmllint disable
                        enabled: false
                    }

                    Controls.Label {
                        id: irSamples

                        Layout.alignment: Qt.AlignHCenter
                        text: convolverPage.pluginBackend ? convolverPage.pluginBackend.kernelSamples : "0"
                        enabled: false
                    }

                    Controls.Label {
                        id: irDuration

                        Layout.alignment: Qt.AlignHCenter
                        text: Number(convolverPage.pluginBackend ? convolverPage.pluginBackend.kernelDuration : 0).toLocaleString(Qt.locale(), 'f', 3) + ` ${i18n("s")}` // qmllint disable
                        enabled: false
                    }
                }
            }
        }

        Kirigami.CardsLayout {
            id: cardLayout

            maximumColumns: 3
            uniformCellWidths: true

            Layout.topMargin: Kirigami.Units.mediumSpacing * 2
            Layout.fillHeight: false

            EeSpinBox {
                id: irWidth

                label: i18n("Stereo width") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                value: convolverPage.pluginDB.irWidth
                from: convolverPage.pluginDB.getMinValue("irWidth")
                to: convolverPage.pluginDB.getMaxValue("irWidth")
                decimals: 2
                stepSize: 0.01
                unit: "%"
                onValueModified: v => {
                    convolverPage.pluginDB.irWidth = value;
                }
            }

            EeSpinBox {
                id: dry

                label: i18n("Dry") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: convolverPage.pluginDB.getMinValue("dry")
                to: convolverPage.pluginDB.getMaxValue("dry")
                value: convolverPage.pluginDB.dry
                decimals: 2
                stepSize: 0.01
                unit: i18n("dB")
                minusInfinityMode: true
                onValueModified: v => {
                    convolverPage.pluginDB.dry = v;
                }
            }

            EeSpinBox {
                id: wet

                label: i18n("Wet") // qmllint disable
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: convolverPage.pluginDB.getMinValue("wet")
                to: convolverPage.pluginDB.getMaxValue("wet")
                value: convolverPage.pluginDB.wet
                decimals: 2
                stepSize: 0.01
                unit: i18n("dB")
                minusInfinityMode: true
                onValueModified: v => {
                    convolverPage.pluginDB.wet = v;
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: convolverPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<strong>${TagsPluginName.Package.zita}</strong>`) // qmllint disable
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
            Layout.leftMargin: Kirigami.Units.mediumSpacing * 2
            Layout.rightMargin: Kirigami.Units.largeSpacing * 8
            color: Kirigami.Theme.disabledTextColor
        }

        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    text: i18n("Autogain") // qmllint disable
                    icon.name: "audio-volume-medium-symbolic"
                    checkable: true
                    checked: convolverPage.pluginDB.autogain
                    onTriggered: {
                        if (checked !== convolverPage.pluginDB.autogain)
                            convolverPage.pluginDB.autogain = checked;
                    }
                },
                Kirigami.Action {
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    text: i18n("Reset") // qmllint disable
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        convolverPage.pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
