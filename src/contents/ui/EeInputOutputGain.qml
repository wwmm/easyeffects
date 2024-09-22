import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    implicitHeight: grid.implicitHeight
    implicitWidth: grid.implicitWidth

    GridLayout {
        id: grid

        anchors.fill: parent
        columns: 3
        rows: 2
        columnSpacing: Kirigami.Units.smallSpacing
        rowSpacing: Kirigami.Units.smallSpacing

        EeSpinBox {
            id: inputGain

            label: i18n("Input")
            from: -36
            to: 36
            // value: pluginDB.inputGain
            decimals: 0
            stepSize: 1
            unit: "dB"
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

        EeSpinBox {
            id: outputGain

            label: i18n("Output")
            from: -36
            to: 36
            // value: pluginDB.outputGain
            decimals: 0
            stepSize: 1
            unit: "dB"
            onValueModified: (v) => {
                pluginDB.outputGain = v;
            }
        }

        EeCircularProgress {
            value: 0.35
        }

        EeCircularProgress {
            value: 0.3
        }

    }

}
