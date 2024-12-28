import QtGraphs
import QtQml
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Item {
    id: widgetRoot

    property int seriesType: 0
    property bool useOpenGL: true
    property real xMin: 0
    property real xMax: 1
    property real yMin: 0
    property real yMax: 1
    readonly property color color1: Kirigami.Theme.negativeTextColor
    readonly property color color2: Kirigami.Theme.alternateBackgroundColor
    readonly property color color3: Qt.darker(color2, 1.1)
    readonly property color backgroundRectColor: Kirigami.Theme.backgroundColor
    property var graphData: [Qt.point(50.5, 0.2), Qt.point(100, 0.4), Qt.point(1000, 0.6), Qt.point(10000, 0.8), Qt.point(15000, 1)]

    function updateData(newData) {
        if (splineSeries.visible === true)
            splineSeries.replace(newData);

    }

    implicitHeight: DB.Manager.spectrum.height
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    ColumnLayout {
        // Row {
        //     Layout.fillWidth: true
        //     Layout.fillHeight: false
        //     Repeater {
        //         id: axisRepeater
        //         model: 10
        //         Controls.Label {
        //             width: chart.width / axisRepeater.count
        //             text: index
        //         }
        //     }
        // }

        id: columnLayout

        anchors.fill: parent

        GraphsView {
            id: chart

            Layout.leftMargin: -55
            antialiasing: true
            marginBottom: -5 // https://github.com/qt/qtgraphs/blob/dev/src/graphs2d/qgraphsview_p.h
            marginTop: 0
            marginLeft: 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            Component.onCompleted: {
                for (let n = 0; n < graphData.length; n++) {
                    splineSeries.append(graphData[n].x, graphData[n].y);
                    scatterSeries.append(graphData[n].x, graphData[n].y);
                    areaLineSeries.append(graphData[n].x, graphData[n].y);
                }
            }

            BarSeries {
                id: barSeries

                visible: seriesType === 0

                BarSet {
                    id: barSeriesSet

                    values: widgetRoot.graphData
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
                visible: seriesType === 3

                upperSeries: LineSeries {
                    id: areaLineSeries

                    visible: seriesType === 1
                }

            }

            axisX: ValueAxis {
                id: axisFreq

                labelFormat: "%.1f"
                min: xMin
                max: xMax
                gridVisible: false
                subGridVisible: false
                lineVisible: false
                labelDecimals: 0
                tickInterval: 0
            }

            axisY: ValueAxis {
                id: axisAmplitude

                labelFormat: "%.1e"
                gridVisible: false
                subGridVisible: false
                lineVisible: false
                visible: false
                labelsVisible: false
                titleVisible: false
                min: yMin
                max: yMax
            }

            theme: GraphsTheme {
                colorScheme: GraphsTheme.ColorScheme.Dark // Light, Dark, Automatic
                theme: GraphsTheme.Theme.OrangeSeries // QtGreen, QtGreenNeon, MixSeries, OrangeSeries, YellowSeries, BlueSeries, PurpleSeries, GreySeries
                plotAreaBackgroundColor: "transparent"
            }

        }

    }

}
