import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: Kirigami.Units.smallSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        RowLayout {
            Controls.Label {
                Layout.alignment: Qt.AlignLeft
                text: i18n("Input")
            }

            Controls.Slider {
                id: inputGain

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                value: 0
                from: -36
                to: 36
                stepSize: 1
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: inputGain.value + " dB"
            }

        }

        GridLayout {
            columns: 2
            rows: 2

            Controls.ProgressBar {
                id: inputLevelLeft

                Layout.fillWidth: true
                from: 0
                to: 1
                value: 0.25
                indeterminate: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: "-1"
            }

            Controls.ProgressBar {
                id: inputLevelRight

                Layout.fillWidth: true
                from: 0
                to: 1
                value: 0.25
                indeterminate: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: "-1"
            }

        }

        RowLayout {
            Controls.Label {
                Layout.alignment: Qt.AlignLeft
                text: i18n("Output")
            }

            Controls.Slider {
                id: outputGain

                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                orientation: Qt.Horizontal
                snapMode: Controls.Slider.SnapAlways
                value: 0
                from: -36
                to: 36
                stepSize: 1
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: outputGain.value + " dB"
            }

        }

        GridLayout {
            columns: 2
            rows: 2

            Controls.ProgressBar {
                id: outputLevelLeft

                Layout.fillWidth: true
                from: 0
                to: 1
                value: 0.25
                indeterminate: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: "-1"
            }

            Controls.ProgressBar {
                id: outputLevelRight

                Layout.fillWidth: true
                from: 0
                to: 1
                value: 0.25
                indeterminate: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignRight
                text: "-1"
            }

        }

    }

}
