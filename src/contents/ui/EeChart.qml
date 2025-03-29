import QtGraphs
import QtQml
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

    function updateData(inputData) {
        //We do not want the object received as argument to be modified here
        let newData = [];
        for (let n = 0; n < inputData.length; n++) {
            newData.push(inputData[n]);
        }
        let minX = Number.POSITIVE_INFINITY;
        let maxX = Number.NEGATIVE_INFINITY;
        let minY = Number.POSITIVE_INFINITY;
        let maxY = Number.NEGATIVE_INFINITY;
        for (let n = 0; n < newData.length; n++) {
            minX = newData[n].x < minX ? newData[n].x : minX;
            maxX = newData[n].x > maxX ? newData[n].x : maxX;
            minY = newData[n].y < minY ? newData[n].y : minY;
            maxY = newData[n].y > maxY ? newData[n].y : maxY;
        }
        if (dynamicXScale === true) {
            xMin = minX;
            xMax = maxX;
        } else {
            xMin = minX < xMin ? minX : xMin;
            xMax = maxX > xMax ? maxX : xMax;
        }
        if (dynamicYScale === true) {
            yMin = minY;
            yMax = maxY;
        } else {
            yMin = minY < yMin ? minY : yMin;
            yMax = maxY > yMax ? maxY : yMax;
        }
        for (let n = 0; n < newData.length; n++) {
            if (logarithimicHorizontalAxis === true)
                newData[n].x = Math.log10(newData[n].x);

            if (logarithimicVerticalAxis === true)
                newData[n].y = Math.log10(newData[n].y);

        }
        if (splineSeries.visible === true)
            splineSeries.replace(newData);

        if (scatterSeries.visible === true)
            scatterSeries.replace(newData);

        if (areaSeries.visible === true)
            areaLineSeries.replace(newData);

        if (barSeries.visible === true) {
            barSeriesSet.clear();
            for (let n = 0; n < newData.length; n++) {
                barSeriesSet.append(newData[n].y);
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
            marginBottom: 0 // https://github.com/qt/qtgraphs/blob/dev/src/graphs2d/qgraphsview_p.h
            marginTop: 0
            marginLeft: 0
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            axisX: {
                if (barSeries.visible === false)
                    return horizontalAxis;
                else
                    return barAxis;
            }

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

                    readonly property real nTicks: 11
                    readonly property real step: {
                        if (logarithimicHorizontalAxis !== true)
                            return (xMax - xMin) / (nTicks - 1);
                        else
                            return (xMaxLog - xMinLog) / (nTicks - 1);
                    }
                    readonly property real labelWidth: widgetRoot.width / (nTicks - 1)

                    model: nTicks

                    Controls.Label {
                        readonly property real value: {
                            if (logarithimicHorizontalAxis !== true)
                                return xMin + index * axisRepeater.step;
                            else
                                return Math.pow(10, xMinLog + index * axisRepeater.step);
                        }

                        padding: 0
                        color: chart.theme.labelTextColor
                        leftPadding: {
                            if (index !== (axisRepeater.count - 1))
                                return 0;
                            else
                                return -axisRepeater.labelWidth - 6 * Kirigami.Units.smallSpacing;
                        }
                        width: axisRepeater.labelWidth
                        text: {
                            if (index !== (axisRepeater.count - 1))
                                return Number(value).toLocaleString(Qt.locale(), 'f', xAxisDecimals);
                            else
                                return xUnit;
                        }
                        horizontalAlignment: {
                            if (index === (axisRepeater.count - 1))
                                return Qt.AlignHCenter;
                            else
                                return Qt.AlignLeft;
                        }
                    }

                }

            }

        }

    }

}
