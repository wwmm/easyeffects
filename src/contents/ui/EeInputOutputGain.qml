import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: control

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

                    label: i18n("Input")
                    labelFillWidth: true
                    from: -36
                    to: 36
                    // value: pluginDB.inputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        pluginDB.inputGain = v;
                    }
                }

                EeCircularProgress {
                    value: 0.2
                }

                EeCircularProgress {
                    value: 0.75
                }

            }

            RowLayout {
                EeSpinBox {
                    id: outputGain

                    label: i18n("Output")
                    labelFillWidth: true
                    from: -36
                    to: 36
                    // value: pluginDB.outputGain
                    decimals: 0
                    stepSize: 1
                    unit: "dB"
                    boxWidth: 5 * Kirigami.Units.gridUnit
                    onValueModified: (v) => {
                        pluginDB.outputGain = v;
                    }
                }

                EeCircularProgress {
                    value: 0.35
                    Layout.alignment: Qt.AlignVCenter
                }

                EeCircularProgress {
                    value: 0.3
                    Layout.alignment: Qt.AlignVCenter
                }

            }

        }

    }

}
