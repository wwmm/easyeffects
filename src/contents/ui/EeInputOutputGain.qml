import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: control

    property real inputGain: 0
    property real outputGain: 0
    property real from: -36
    property real to: 36
    property alias inputLevelLeft: inputLevelLeft.value
    property alias inputLevelRight: inputLevelRight.value
    property alias outputLevelLeft: outputLevelLeft.value
    property alias outputLevelRight: outputLevelRight.value

    implicitHeight: grid.implicitHeight
    implicitWidth: grid.implicitWidth

    ColumnLayout {
        anchors.fill: parent

        Kirigami.CardsLayout {
            id: grid

            Layout.fillWidth: true

            RowLayout {
                EeSpinBox {
                    id: inputGain

                    Layout.alignment: Qt.AlignLeft
                    label: i18n("Input")
                    labelFillWidth: true
                    from: control.from
                    to: control.to
                    value: control.inputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        control.inputGain = v;
                    }
                }

                EeCircularProgress {
                    id: inputLevelLeft

                    Layout.alignment: Qt.AlignRight
                    implicitWidth: 5 * Kirigami.Units.largeSpacing
                    implicitHeight: 5 * Kirigami.Units.largeSpacing
                    from: -100
                    to: 10
                    value: 0
                    decimals: 0
                }

                EeCircularProgress {
                    id: inputLevelRight

                    Layout.alignment: Qt.AlignRight
                    value: 0.75
                }

            }

            RowLayout {
                EeSpinBox {
                    id: outputGain

                    label: i18n("Output")
                    labelFillWidth: true
                    from: control.from
                    to: control.to
                    value: control.outputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        control.outputGain = v;
                    }
                }

                EeCircularProgress {
                    id: outputLevelLeft

                    value: 0.35
                    Layout.alignment: Qt.AlignVCenter
                }

                EeCircularProgress {
                    id: outputLevelRight

                    value: 0.3
                    Layout.alignment: Qt.AlignVCenter
                }

            }

        }

    }

}
