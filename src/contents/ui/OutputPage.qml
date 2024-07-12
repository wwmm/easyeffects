import QtGraphs
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    // header: EeChart {
    //     seriesType: EEdbSpectrum.spectrumShape
    //     useOpenGL: EEdbSpectrum.useOpenGL
    // }

    header: Rectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.Header
        color: Kirigami.Theme.backgroundColor
        implicitHeight: EEdbSpectrum.height * 2

        anchors {
            left: parent.left
            leftMargin: 0
            right: parent.right
            rightMargin: 0
            top: parent.top
            topMargin: 0
        }

        GraphsView {
            anchors.fill: parent
            marginBottom: 10
            marginTop: 10

            LineSeries {
                id: lineSeries

                theme: seriesTheme

                XYPoint {
                    x: 0
                    y: 0
                }

                XYPoint {
                    x: 10
                    y: 2.1
                }

                XYPoint {
                    x: 20
                    y: 3.3
                }

                XYPoint {
                    x: 30
                    y: 2.1
                }

                XYPoint {
                    x: 40
                    y: 4.9
                }

                XYPoint {
                    x: 50
                    y: 3
                }

                axisX: ValueAxis {
                    max: 60
                    labelDecimals: 1
                }

                axisY: ValueAxis {
                    max: 10
                    labelDecimals: 1
                    visible: true
                }

            }

            SeriesTheme {
                id: seriesTheme

                colors: [Kirigami.Theme.linkColor, Kirigami.Theme.negativeTextColor]
            }

            theme: GraphTheme {
                readonly property color c1: Kirigami.Theme.negativeTextColor
                readonly property color c2: Kirigami.Theme.alternateBackgroundColor
                readonly property color c3: Qt.darker(c2, 1.1)

                // colorTheme: GraphTheme.ColorThemeDark
                gridMajorBarsColor: c3
                gridMinorBarsColor: c2
                axisXMajorColor: c3
                axisYMajorColor: c3
                axisXMinorColor: c2
                axisYMinorColor: c2
                axisXLabelsColor: c1
                axisYLabelsColor: c1
            }

        }

    }

    footer: FooterInputOutput {
        streamsTabIconName: "multimedia-player-symbolic"
    }

}
