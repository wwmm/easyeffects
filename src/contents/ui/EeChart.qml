/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound
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
    property int xAxisDecimals: 0
    property int yAxisDecimals: 0
    property bool logarithmicHorizontalAxis: true
    property bool logarithmicVerticalAxis: false
    property bool dynamicXScale: true
    property bool dynamicYScale: true
    property bool baselineLogMode: false
    property real xMin: 0
    property real xMax: 1
    property real yMin: 0
    property real yMax: 1
    property real yDataOffset: 0
    property string xUnit: ""
    property string yUnit: ""
    property list<point> baselineCache: []

    readonly property real xMinLog: Math.log10(xMin)
    readonly property real xMaxLog: Math.log10(xMax)
    readonly property real yMinLog: Math.log10(yMin)
    readonly property real yMaxLog: Math.log10(yMax)
    readonly property color backgroundRectColor: Kirigami.Theme.backgroundColor
    readonly property int targetTicks: Math.max(2, Math.floor(width / (Kirigami.Units.gridUnit * 6)))
    readonly property int coordLabelOffset: Kirigami.Units.smallSpacing

    implicitHeight: columnLayout.implicitHeight
    implicitWidth: columnLayout.implicitWidth
    Kirigami.Theme.colorSet: Kirigami.Theme.View

    readonly property list<real> linearTicks: {
        const step = (xMax - xMin) / targetTicks;

        const start = xMin;

        const ticks = [];

        for (let v = start; v <= xMax; v += step) {
            ticks.push(v);
        }

        return ticks;
    }

    readonly property list<real> logTicks: {
        const step = (xMaxLog - xMinLog) / targetTicks;

        const ticks = [];

        for (let p = xMinLog; p <= xMaxLog; p += step) {
            ticks.push(Math.pow(10, p));
        }

        return ticks;
    }

    function updateData(inputData: list<point>) {
        if (!inputData || inputData.length === 0) {
            return;
        }

        let minX = Number.POSITIVE_INFINITY;
        let maxX = Number.NEGATIVE_INFINITY;
        let minY = Number.POSITIVE_INFINITY;
        let maxY = Number.NEGATIVE_INFINITY;

        let processedData = [];
        processedData.length = inputData.length;

        for (let n = 0; n < inputData.length; n++) {
            let y = inputData[n].y + yDataOffset;
            let x = inputData[n].x;

            minX = Math.min(minX, x);
            maxX = Math.max(maxX, x);
            minY = Math.min(minY, y);
            maxY = Math.max(maxY, y);

            processedData[n] = Qt.point(widgetRoot.logarithmicHorizontalAxis ? Math.log10(x) : x, widgetRoot.logarithmicVerticalAxis ? Math.log10(y) : y);
        }

        // Avoid tiny changes triggering expensive relayouts
        const epsilon = 0.01; // 1% threshold
        const xRange = (maxX - minX) || 1;
        const yRange = (maxY - minY) || 1;

        if (dynamicXScale) {
            if (Math.abs(minX - xMin) / xRange > epsilon) {
                xMin = minX;
            }

            if (Math.abs(maxX - xMax) / xRange > epsilon) {
                xMax = maxX;
            }
        } else {
            const newXMin = Math.min(minX, xMin);
            const newXMax = Math.max(maxX, xMax);

            if (Math.abs(newXMin - xMin) / xRange > epsilon) {
                xMin = newXMin;
            }

            if (Math.abs(newXMax - xMax) / xRange > epsilon) {
                xMax = newXMax;
            }
        }

        if (dynamicYScale) {
            if (Math.abs(minY - yMin) / yRange > epsilon) {
                yMin = minY;
            }

            if (Math.abs(maxY - yMax) / yRange > epsilon) {
                yMax = maxY;
            }
        } else {
            const newYMin = Math.min(minY, yMin);
            const newYMax = Math.max(maxY, yMax);

            if (Math.abs(newYMin - yMin) / yRange > epsilon) {
                yMin = newYMin;
            }
            if (Math.abs(newYMax - yMax) / yRange > epsilon) {
                yMax = newYMax;
            }
        }

        if (splineSeries.visible === true) {
            splineSeries.replace(processedData);
        }

        if (scatterSeries.visible === true)
            scatterSeries.replace(processedData);

        if (areaSeries.visible === true) {
            areaLineSeries.replace(processedData);

            // For some reason letting QtGraphs use a baseline over the x axis causes graphical artifacts
            // Reuse baseline array, only regenerate when size or log mode changes

            if (!widgetRoot.baselineCache || widgetRoot.baselineCache.length !== processedData.length || widgetRoot.baselineLogMode !== widgetRoot.logarithmicVerticalAxis) {
                let baseline = [];

                baseline.length = processedData.length;

                const baselineY = widgetRoot.logarithmicVerticalAxis ? Math.log10(1e-12) : -2;

                for (let n = 0; n < processedData.length; n++) {
                    baseline[n] = Qt.point(processedData[n].x, baselineY);
                }

                widgetRoot.baselineCache = baseline;
                widgetRoot.baselineLogMode = widgetRoot.logarithmicVerticalAxis;
            }

            areaBaseline.replace(widgetRoot.baselineCache);
        }

        if (barSeries.visible === true) {
            barSeriesSet.clear();
            for (let n = 0; n < processedData.length; n++) {
                barSeriesSet.append(processedData[n].y);
            }
        }
    }

    function clearData() {
        splineSeries.clear();
        scatterSeries.clear();
        barSeries.clear();
        areaLineSeries.clear();
    }

    function mapToValueX(mouseX: real): real {
        if (chart.plotArea.width <= 0) {
            return 0;
        }

        const normalizedX = (mouseX - chart.plotArea.x) / chart.plotArea.width;

        if (logarithmicHorizontalAxis) {
            return Math.pow(10, horizontalAxis.min + normalizedX * (horizontalAxis.max - horizontalAxis.min));
        } else {
            return horizontalAxis.min + normalizedX * (horizontalAxis.max - horizontalAxis.min);
        }
    }

    function mapToValueY(mouseY: real): real {
        if (chart.plotArea.height <= 0) {
            return 0;
        }

        const normalizedY = 1 - (mouseY - chart.plotArea.y) / chart.plotArea.height;

        if (logarithmicVerticalAxis) {
            return Math.pow(10, verticalAxis.min + normalizedY * (verticalAxis.max - verticalAxis.min));
        } else {
            return verticalAxis.min + normalizedY * (verticalAxis.max - verticalAxis.min);
        }
    }

    GraphsTheme {
        id: qtTheme

        colorScheme: widgetRoot.colorScheme
        theme: widgetRoot.colorTheme

        onColorSchemeChanged: {
            axisRectangle.color = chart.theme.labelBackgroundColor;
        }

        onThemeChanged: {
            axisRectangle.color = chart.theme.labelBackgroundColor;
        }

        // Component.onCompleted: {
        //     console.log("plot area: " + plotAreaBackgroundColor);
        //     console.log("backgroundColor: " + backgroundColor);
        //     console.log("seriesColors: " + seriesColors);
        //     console.log("labelTextColor: " + labelTextColor);
        //     console.log("labelBackgroundColor: " + labelBackgroundColor);
        //     console.log("borderColors: " + borderColors);
        // }
    }

    GraphsTheme {
        id: userTheme

        theme: GraphsTheme.Theme.UserDefined
        backgroundColor: DbGraph.backgroundColor
        plotAreaBackgroundColor: DbGraph.plotAreaBackgroundColor
        seriesColors: [DbGraph.seriesColors]
        labelTextColor: DbGraph.labelTextColor
        labelBackgroundColor: DbGraph.labelBackgroundColor
        borderColors: [DbGraph.borderColors]

        Connections {
            /*
             * For some strange reason qt graphs properties are not updated when the corresponding database property
             * changes. Probably a qt graph bug where the binding is broken internally.
             */

            target: DbGraph

            function onBackgroundColorChanged() {
                userTheme.backgroundColor = DbGraph.backgroundColor;
            }

            function onPlotAreaBackgroundColorChanged() {
                userTheme.plotAreaBackgroundColor = DbGraph.plotAreaBackgroundColor;
            }

            function onSeriesColorsChanged() {
                userTheme.seriesColors = [DbGraph.seriesColors];
            }

            function onLabelTextColorChanged() {
                userTheme.labelTextColor = DbGraph.labelTextColor;
            }

            function onLabelBackgroundColorChanged() {
                userTheme.labelBackgroundColor = DbGraph.labelBackgroundColor;

                axisRectangle.color = DbGraph.labelBackgroundColor;
            }

            function onBorderColorsChanged() {
                userTheme.borderColors = [DbGraph.borderColors];
            }
        }
    }

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
                visible: widgetRoot.seriesType === 0
                BarSet {
                    id: barSeriesSet
                }
            }

            SplineSeries {
                id: splineSeries
                visible: widgetRoot.seriesType === 1

                width: DbGraph.lineWidth
            }
            ScatterSeries {
                id: scatterSeries
                visible: widgetRoot.seriesType === 2
            }

            AreaSeries {
                id: areaSeries
                visible: widgetRoot.seriesType === 3
                upperSeries: SplineSeries {
                    id: areaLineSeries

                    width: DbGraph.lineWidth
                }

                lowerSeries: LineSeries {
                    id: areaBaseline
                }
            }

            ValueAxis {
                id: horizontalAxis
                labelFormat: "%.1f"
                min: widgetRoot.logarithmicHorizontalAxis !== true ? widgetRoot.xMin : widgetRoot.xMinLog
                max: widgetRoot.logarithmicHorizontalAxis !== true ? widgetRoot.xMax : widgetRoot.xMaxLog
                gridVisible: DbGraph.gridVisible
                subGridVisible: DbGraph.gridVisible
                lineVisible: false
                visible: false
                labelDecimals: 0
            }

            BarCategoryAxis {
                id: barAxis
                categories: [2024, 2025, 2026]
                gridVisible: DbGraph.gridVisible
                subGridVisible: DbGraph.gridVisible
                visible: false
                lineVisible: false
            }

            axisY: ValueAxis {
                id: verticalAxis
                labelFormat: "%.1e"
                gridVisible: DbGraph.gridVisible
                subGridVisible: DbGraph.gridVisible
                lineVisible: false
                visible: false
                labelsVisible: false
                titleVisible: false
                min: widgetRoot.logarithmicVerticalAxis !== true ? widgetRoot.yMin : widgetRoot.yMinLog
                max: widgetRoot.logarithmicVerticalAxis !== true ? widgetRoot.yMax : widgetRoot.yMaxLog
            }

            theme: widgetRoot.colorTheme !== GraphsTheme.Theme.UserDefined ? qtTheme : userTheme
        }

        Rectangle {
            id: axisRectangle
            Layout.fillWidth: true
            Layout.fillHeight: false
            color: chart.theme.labelBackgroundColor
            implicitHeight: axisRow.implicitHeight

            Row {
                id: axisRow
                padding: 0
                x: chart.plotArea.x  // Align with plot area start
                width: chart.plotArea.width  // Match plot area width

                Repeater {
                    id: axisRepeater

                    readonly property var tickValues: widgetRoot.logarithmicHorizontalAxis ? widgetRoot.logTicks : widgetRoot.linearTicks

                    model: tickValues.length

                    Controls.Label {
                        required property int index
                        readonly property real value: index < axisRepeater.tickValues.length ? axisRepeater.tickValues[index] : 0

                        width: widgetRoot.width / widgetRoot.targetTicks
                        padding: 0
                        color: chart.theme.labelTextColor
                        text: value < 1000 ? Number(value).toLocaleString(Qt.locale(), 'f', widgetRoot.xAxisDecimals) : `${Number(value / 1000).toLocaleString(Qt.locale(), 'f', 1)}${Units.k}`
                        horizontalAlignment: Qt.AlignLeft
                        visible: index < (axisRepeater.tickValues.length - 1)
                    }
                }
            }

            Controls.Label {
                text: widgetRoot.xUnit
                padding: 0
                color: chart.theme.labelTextColor
                horizontalAlignment: Qt.AlignRight
                anchors.right: parent.right
            }
        }
    }

    // Coordinate display label
    Controls.Label {
        id: coordinateLabel
        visible: false
        padding: Kirigami.Units.smallSpacing
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            border.color: Kirigami.Theme.textColor
            border.width: 1
            radius: Kirigami.Units.smallSpacing
            opacity: 0.9
        }
        color: Kirigami.Theme.textColor
        font.pointSize: Kirigami.Theme.smallFont.pointSize
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: function (mouse) {
            // Calculating the label x coordinate

            let labelX = mouse.x + widgetRoot.coordLabelOffset;

            if (labelX + coordinateLabel.width > widgetRoot.width) {
                labelX = widgetRoot.width - coordinateLabel.width - widgetRoot.coordLabelOffset;
            } else if (x < 0) {
                labelX = widgetRoot.coordLabelOffset;
            }

            coordinateLabel.x = labelX;

            // Calculating the y coordinate

            let labelY = mouse.y - coordinateLabel.height - widgetRoot.coordLabelOffset;

            if (labelY < 0) {
                labelY = widgetRoot.coordLabelOffset;
            } else if (labelY + coordinateLabel.height > widgetRoot.height) {
                labelY = widgetRoot.height - coordinateLabel.height - widgetRoot.coordLabelOffset;
            }

            coordinateLabel.y = labelY;

            const dataX = widgetRoot.mapToValueX(mouse.x);
            const dataY = widgetRoot.mapToValueY(mouse.y) - widgetRoot.yDataOffset;

            // const newText = `${Number(dataX).toLocaleString(Qt.locale(), 'f', widgetRoot.xAxisDecimals)} ${widgetRoot.xUnit}`;
            const newText = `x: ${Number(dataX).toLocaleString(Qt.locale(), 'f', widgetRoot.xAxisDecimals)} ${widgetRoot.xUnit}, y: ${Number(dataY).toLocaleString(locale, 'f', widgetRoot.yAxisDecimals)} ${widgetRoot.yUnit}`;

            if (coordinateLabel.text !== newText) {
                coordinateLabel.text = newText;
            }

            coordinateLabel.visible = true;
        }
        onExited: {
            coordinateLabel.visible = false;
        }
    }
}
