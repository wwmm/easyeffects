// bug: https://bugreports.qt.io/browse/QTBUG-66150

import QtGraphs
import QtQml
import QtQuick
import ee.database as DB
import org.kde.kirigami as Kirigami

Item {
    id: widgetRoot

    property int seriesType: 0
    property bool useOpenGL: true
    readonly property color color1: Kirigami.Theme.negativeTextColor
    readonly property color color2: Kirigami.Theme.alternateBackgroundColor
    readonly property color color3: Qt.darker(color2, 1.1)
    readonly property color backgroundRectColor: Kirigami.Theme.backgroundColor
    property var testData: [Qt.point(50.5, 0.2), Qt.point(100, 0.4), Qt.point(1000, 0.6), Qt.point(10000, 0.8), Qt.point(15000, 1)]

    implicitHeight: DB.Manager.spectrum.height
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    // rectangle used as workaround for the bug linked at the top of this file
    Rectangle {
        implicitHeight: DB.Manager.spectrum.height
        color: backgroundRectColor

        anchors {
            left: parent.left
            leftMargin: 0
            right: parent.right
            rightMargin: 0
            top: parent.top
            topMargin: 0
        }

    }

    GraphsView {
        // LogValueAxis {
        //     id: axisFreqLog
        //     labelFormat: "%.0f"
        //     min: DB.Manager.spectrum.minimumFrequency
        //     max: DB.Manager.spectrum.maximumFrequency
        //     base: 10
        //     gridVisible: false
        //     lineVisible: false
        //     labelsColor: color1
        //     gridLineColor: color3
        //     minorGridLineColor: color2
        // }

        id: chart

        implicitHeight: DB.Manager.spectrum.height
        antialiasing: true
        marginBottom: -5 // https://github.com/qt/qtgraphs/blob/dev/src/graphs2d/qgraphsview_p.h
        marginTop: 0
        marginLeft: 0
        axisX: axisFreq
        axisY: axisAmplitude
        Component.onCompleted: {
            for (let n = 0; n < testData.length; n++) {
                splineSeries.append(testData[n].x, testData[n].y);
                scatterSeries.append(testData[n].x, testData[n].y);
                areaLineSeries.append(testData[n].x, testData[n].y);
            }
        }

        anchors {
            fill: parent
            leftMargin: -60
            rightMargin: 0
            topMargin: 0
            bottomMargin: 0
        }

        ValueAxis {
            id: axisFreq

            labelFormat: "%.1f"
            min: DB.Manager.spectrum.minimumFrequency
            max: DB.Manager.spectrum.maximumFrequency
            gridVisible: false
            subGridVisible: false
            lineVisible: false
            labelDecimals: 0
        }

        ValueAxis {
            id: axisAmplitude

            labelFormat: "%.1e"
            gridVisible: false
            subGridVisible: false
            lineVisible: false
            visible: false
            labelsVisible: false
            titleVisible: false
        }

        BarSeries {
            id: barSeries

            visible: seriesType === 0

            BarSet {
                id: barSeriesSet

                values: widgetRoot.testData
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

        theme: GraphsTheme {
            colorScheme: GraphsTheme.ColorScheme.Dark // Light, Dark, Automatic
            theme: GraphsTheme.Theme.OrangeSeries // QtGreen, QtGreenNeon, MixSeries, OrangeSeries, YellowSeries, BlueSeries, PurpleSeries, GreySeries
            plotAreaBackgroundColor: "transparent"
        }

    }

}
