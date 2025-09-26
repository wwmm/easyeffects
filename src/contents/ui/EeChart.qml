import QtGraphs
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: widgetRoot

    property int seriesType: 0
    property int colorScheme: GraphsTheme.ColorScheme.Dark
    property int colorTheme: GraphsTheme.Theme.QtGreenNeon
    property bool logarithimicHorizontalAxis: true
    property bool logarithimicVerticalAxis: false
    property bool dynamicXScale: true
    property bool dynamicYScale: true
    property real xMin: 0
    property real xMax: 1
    property real yMin: 0
    property real yMax: 1
    property string xUnit: ""
    property int xAxisDecimals: 0
    readonly property real xMinLog: Math.log10(xMin)
    readonly property real xMaxLog: Math.log10(xMax)
    readonly property real yMinLog: Math.log10(yMin)
    readonly property real yMaxLog: Math.log10(yMax)
    readonly property color backgroundRectColor: Kirigami.Theme.backgroundColor
    readonly property int targetTicks: Math.max(2, Math.floor(width / (Kirigami.Units.gridUnit * 6)))

    readonly property var linearTicks: {
        const step = (xMax - xMin) / targetTicks;

        const start = xMin;

        const ticks = [];

        for (let v = start; v <= xMax; v += step) {
            ticks.push(v);
        }

        return ticks;
    }

    readonly property var logTicks: {
        const step = (xMaxLog - xMinLog) / targetTicks;

        const ticks = [];

        for (let p = xMinLog; p <= xMaxLog; p += step) {
            ticks.push(Math.pow(10, p));
        }

        return ticks;
    }

    function updateData(inputData) {
        if (!inputData || inputData.length === 0) {
            return;
        }

        let minX = Number.POSITIVE_INFINITY;
        let maxX = Number.NEGATIVE_INFINITY;
        let minY = Number.POSITIVE_INFINITY;
        let maxY = Number.NEGATIVE_INFINITY;

        for (let n = 0; n < inputData.length; n++) {
            const point = inputData[n];

            minX = Math.min(minX, point.x);
            maxX = Math.max(maxX, point.x);
            minY = Math.min(minY, point.y);
            maxY = Math.max(maxY, point.y);
        }

        if (dynamicXScale) {
            xMin = minX;
            xMax = maxX;
        } else {
            xMin = Math.min(minX, xMin);
            xMax = Math.max(maxX, xMax);
        }

        if (dynamicYScale) {
            yMin = minY;
            yMax = maxY;
        } else {
            yMin = Math.min(minY, yMin);
            yMax = Math.max(maxY, yMax);
        }

        //We do not want the object received as argument to be modified here
        let processedData = Array.from(inputData);

        for (let n = 0; n < processedData.length; n++) {
            const point = inputData[n];

            processedData[n].x = logarithimicHorizontalAxis ? Math.log10(point.x) : point.x;
            processedData[n].y = logarithimicVerticalAxis ? Math.log10(point.y) : point.y;
        }

        if (splineSeries.visible === true)
            splineSeries.replace(processedData);

        if (scatterSeries.visible === true)
            scatterSeries.replace(processedData);

        if (areaSeries.visible === true)
            areaLineSeries.replace(processedData);

        if (barSeries.visible === true) {
            barSeriesSet.clear();
            for (let n = 0; n < processedData.length; n++) {
                barSeriesSet.append(processedData[n].y);
            }
        }
    }

    implicitHeight: columnLayout.implicitHeight
    implicitWidth: columnLayout.implicitWidth
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    ColumnLayout {
        id: columnLayout

        anchors.fill: parent
        spacing: 0

        GraphsView {
            id: chart

            antialiasing: true
            marginBottom: 0
            marginTop: 0
            marginLeft: 0
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: barSeries.visible ? barAxis : horizontalAxis

            BarSeries {
                id: barSeries
                visible: seriesType === 0
                BarSet {
                    id: barSeriesSet
                }
            }

            SplineSeries {
                id: splineSeries
                visible: seriesType === 1
            }
            ScatterSeries {
                id: scatterSeries
                visible: seriesType === 2
            }

            AreaSeries {
                id: areaSeries
                visible: seriesType === 3
                upperSeries: LineSeries {
                    id: areaLineSeries
                }
            }

            ValueAxis {
                id: horizontalAxis
                labelFormat: "%.1f"
                min: logarithimicHorizontalAxis !== true ? xMin : xMinLog
                max: logarithimicHorizontalAxis !== true ? xMax : xMaxLog
                gridVisible: false
                subGridVisible: false
                lineVisible: false
                visible: false
                labelDecimals: 0
            }

            BarCategoryAxis {
                id: barAxis
                categories: [2024, 2025, 2026]
                gridVisible: false
                subGridVisible: false
                visible: false
                lineVisible: false
            }

            axisY: ValueAxis {
                id: verticalAxis
                labelFormat: "%.1e"
                gridVisible: false
                subGridVisible: false
                lineVisible: false
                visible: false
                labelsVisible: false
                titleVisible: false
                min: logarithimicVerticalAxis !== true ? yMin : yMinLog
                max: logarithimicVerticalAxis !== true ? yMax : yMaxLog
            }

            theme: GraphsTheme {
                colorScheme: widgetRoot.colorScheme
                theme: widgetRoot.colorTheme
                plotAreaBackgroundColor: "transparent"
                onColorSchemeChanged: {
                    plotAreaBackgroundColor = "transparent";
                    axisRectangle.color = chart.theme.backgroundColor;
                }
                onThemeChanged: {
                    plotAreaBackgroundColor = "transparent";
                    axisRectangle.color = chart.theme.backgroundColor;
                }
            }
        }

        Rectangle {
            id: axisRectangle
            Layout.fillWidth: true
            Layout.fillHeight: false
            color: chart.theme.backgroundColor
            implicitHeight: axisRow.implicitHeight

            Row {
                id: axisRow
                padding: 0

                Repeater {
                    id: axisRepeater

                    readonly property var tickValues: logarithimicHorizontalAxis ? widgetRoot.logTicks : widgetRoot.linearTicks

                    model: tickValues.length

                    Controls.Label {
                        readonly property real value: axisRepeater.tickValues[index]
                        width: widgetRoot.width / widgetRoot.targetTicks
                        padding: 0
                        color: chart.theme.labelTextColor
                        text: Number(value).toLocaleString(Qt.locale(), 'f', xAxisDecimals)
                        horizontalAlignment: Qt.AlignLeft
                    }
                }
            }

            Controls.Label {
                text: xUnit
                padding: 0
                color: chart.theme.labelTextColor
                horizontalAlignment: Qt.AlignRight
                anchors.right: parent.right
            }
        }
    }
}
