import QtCharts
import QtQml
import QtQuick

ChartView {
    id: widgetRoot

    property int seriesType: 0
    property bool useOpenGL: true
    property var testData: [Qt.point(50.5, 0.2), Qt.point(100, 0.4), Qt.point(1000, 0.6), Qt.point(10000, 0.8), Qt.point(15000, 1)]

    implicitHeight: EEdbSpectrum.height
    antialiasing: true
    theme: EEdbSpectrum.theme
    localizeNumbers: true
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

    margins {
        left: 0
        right: 0
        top: 0
        bottom: 0
    }

    LogValueAxis {
        id: axisFreqLog

        labelFormat: "%.0f"
        min: EEdbSpectrum.minimumFrequency
        max: EEdbSpectrum.maximumFrequency
        base: 10
        gridVisible: false
        lineVisible: false
    }

    ValueAxis {
        id: axisFreq

        labelFormat: "%.1f"
        min: EEdbSpectrum.minimumFrequency
        max: EEdbSpectrum.maximumFrequency
        gridVisible: false
        lineVisible: false
    }

    ValueAxis {
        id: axisY

        labelFormat: "%.1e"
        gridVisible: false
        lineVisible: false
        visible: false
        labelsVisible: false
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
