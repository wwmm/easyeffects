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
    property real xMin: 0
    property real xMax: 1
    property real yMin: 0
    property real yMax: 1
    readonly property real xMinLog: Math.log10(xMin)
    readonly property real xMaxLog: Math.log10(xMax)
    readonly property real yMinLog: Math.log10(yMin)
    readonly property real yMaxLog: Math.log10(yMin)
    readonly property color color1: Kirigami.Theme.negativeTextColor
    readonly property color color2: Kirigami.Theme.alternateBackgroundColor
    readonly property color color3: Qt.darker(color2, 1.1)
    readonly property color backgroundRectColor: Kirigami.Theme.backgroundColor
    property var inputData: []

    function updateData(newData) {
        inputData = newData;
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

    }

    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    ColumnLayout {
        id: columnLayout

        anchors.fill: parent
        spacing: 0

        GraphsView {
            id: chart

            Layout.leftMargin: -60
            antialiasing: true
            marginBottom: -40 // https://github.com/qt/qtgraphs/blob/dev/src/graphs2d/qgraphsview_p.h
            marginTop: 0
            marginLeft: 0
            marginRight: 0
            Layout.fillWidth: true
            Layout.fillHeight: true

            BarSeries {
                id: barSeries

                visible: seriesType === 0

                BarSet {
                    // values: widgetRoot.graphData

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

            axisX: ValueAxis {
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
                }
                onThemeChanged: {
                    plotAreaBackgroundColor = "transparent";
                }
            }

        }

        Row {
            Layout.fillWidth: true
            Layout.fillHeight: false
            padding: 0
            visible: true

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
                            return xMin + (index) * axisRepeater.step;
                        else
                            return Math.pow(10, xMinLog + (index) * axisRepeater.step);
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
                            return Number(value).toLocaleString(Qt.locale(), 'f', 0);
                        else
                            return "Hz";
                    }
                    horizontalAlignment: {
                        if (index === (axisRepeater.count - 1))
                            return Qt.AlignHCenter;
                        else
                            return Qt.AlignLeft;
                    }

                    background: Rectangle {
                        color: chart.theme.backgroundColor
                    }

                }

            }

        }

    }

}
