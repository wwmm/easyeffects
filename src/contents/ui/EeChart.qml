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
    readonly property int coordLabelOffset: Kirigami.Units.smallSpacing

    implicitHeight: columnLayout.implicitHeight
    implicitWidth: columnLayout.implicitWidth
    Kirigami.Theme.colorSet: Kirigami.Theme.View

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

        if (splineSeries.visible === true) {
            splineSeries.replace(processedData);
        }

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

    function clearData() {
        splineSeries.clear();
        scatterSeries.clear();
        barSeries.clear();
        areaLineSeries.clear();
    }

    function mapToValueX(mouseX) {
        if (chart.plotArea.width <= 0) {
            return 0;
        }

        const normalizedX = (mouseX - chart.plotArea.x) / chart.plotArea.width;

        if (logarithimicHorizontalAxis) {
            return Math.pow(10, horizontalAxis.min + normalizedX * (horizontalAxis.max - horizontalAxis.min));
        } else {
            return horizontalAxis.min + normalizedX * (horizontalAxis.max - horizontalAxis.min);
        }
    }

    function mapToValueY(mouseY) {
        if (chart.plotArea.height <= 0) {
            return 0;
        }

        const normalizedY = 1 - (mouseY - chart.plotArea.y) / chart.plotArea.height;

        if (logarithimicVerticalAxis) {
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
                upperSeries: LineSeries {
                    id: areaLineSeries

                    width: DbGraph.lineWidth
                }
            }

            ValueAxis {
                id: horizontalAxis
                labelFormat: "%.1f"
                min: widgetRoot.logarithimicHorizontalAxis !== true ? widgetRoot.xMin : widgetRoot.xMinLog
                max: widgetRoot.logarithimicHorizontalAxis !== true ? widgetRoot.xMax : widgetRoot.xMaxLog
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
                min: widgetRoot.logarithimicVerticalAxis !== true ? widgetRoot.yMin : widgetRoot.yMinLog
                max: widgetRoot.logarithimicVerticalAxis !== true ? widgetRoot.yMax : widgetRoot.yMaxLog
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

                    readonly property var tickValues: widgetRoot.logarithimicHorizontalAxis ? widgetRoot.logTicks : widgetRoot.linearTicks

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
            // const dataY = widgetRoot.mapToValueY(mouse.y);

            const newText = `${Number(dataX).toLocaleString(Qt.locale(), 'f', widgetRoot.xAxisDecimals)} ${widgetRoot.xUnit}`;
            // const newText = `x: ${Number(dataX).toLocaleString(Qt.locale(), 'f', widgetRoot.xAxisDecimals)} ${Units.hz}, y: ${Number(dataY).toLocaleString(locale, 'f', widgetRoot.xAxisDecimals)}`;

            if (coordinateLabel.text !== newText) {
                coordinateLabel.text = newText;
            }

            coordinateLabel.visible = true;
        }
        onExited: {
            coordinateLabel.visible = false;
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
}
