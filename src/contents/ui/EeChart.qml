// bug: https://bugreports.qt.io/browse/QTBUG-66150

import EEdbm
import QtCharts
import QtQml
import QtQuick
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

    implicitHeight: EEdbm.spectrum.height
    Kirigami.Theme.inherit: false
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    // rectangle used as workaround for the bug linked at the top of this file
    Rectangle {
        implicitHeight: EEdbm.spectrum.height
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

    ChartView {
        id: chart

        implicitHeight: EEdbm.spectrum.height
        antialiasing: true
        localizeNumbers: true
        backgroundColor: backgroundRectColor
        backgroundRoundness: 0
        legend.visible: false
        dropShadowEnabled: false
        Component.onCompleted: {
            for (let n = 0; n < testData.length; n++) {
                splineSeries.append(testData[n].x, testData[n].y);
                scatterSeries.append(testData[n].x, testData[n].y);
                areaLineSeries.append(testData[n].x, testData[n].y);
            }
        }

        anchors {
            left: parent.left
            leftMargin: 0
            right: parent.right
            rightMargin: 0
            top: parent.top
            topMargin: 0
        }

        margins {
            left: 0
            right: 0
            top: 0
            bottom: 0
        }

        LogValueAxis {
            id: axisFreqLog

            labelFormat: "%.0f"
            min: EEdbm.spectrum.minimumFrequency
            max: EEdbm.spectrum.maximumFrequency
            base: 10
            gridVisible: false
            lineVisible: false
            labelsColor: color1
            gridLineColor: color3
            minorGridLineColor: color2
        }

        ValueAxis {
            id: axisFreq

            labelFormat: "%.1f"
            min: EEdbm.spectrum.minimumFrequency
            max: EEdbm.spectrum.maximumFrequency
            gridVisible: false
            lineVisible: false
            labelsColor: color1
            gridLineColor: color3
            minorGridLineColor: color2
        }

        ValueAxis {
            id: axisY

            labelFormat: "%.1e"
            gridVisible: false
            lineVisible: false
            visible: false
            labelsVisible: false
            labelsColor: color1
            gridLineColor: color3
            minorGridLineColor: color2
        }

        BarSeries {
            id: barSeries

            axisX: axisFreqLog
            axisY: axisY
            useOpenGL: useOpenGL
            visible: seriesType === 0

            BarSet {
                id: barSeriesSet

                values: widgetRoot.testData
                onClicked: console.debug("clicked!" + index + " " + this.at(index))
            }

        }

        SplineSeries {
            id: splineSeries

            axisX: axisFreqLog
            axisY: axisY
            useOpenGL: useOpenGL
            visible: seriesType === 1
        }

        ScatterSeries {
            id: scatterSeries

            axisX: axisFreqLog
            axisY: axisY
            useOpenGL: useOpenGL
            visible: seriesType === 2
        }

        AreaSeries {
            axisX: axisFreqLog
            useOpenGL: useOpenGL
            visible: seriesType === 3

            upperSeries: LineSeries {
                id: areaLineSeries

                axisX: axisFreqLog
                axisY: axisY
                useOpenGL: useOpenGL
                visible: seriesType === 1
            }

        }

    }

}
