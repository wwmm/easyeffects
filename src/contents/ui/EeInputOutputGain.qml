import "Common.js" as Common
import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Item {
    id: control

    required property var pluginDB
    property alias inputLevelLeft: inputRow.leftLevel
    property alias inputLevelRight: inputRow.rightLevel
    property alias outputLevelLeft: outputRow.leftLevel
    property alias outputLevelRight: outputRow.rightLevel
    readonly property real radius: 2.5 * Kirigami.Units.gridUnit

    implicitHeight: column.implicitHeight
    implicitWidth: column.implicitWidth

    Accessible.role: Accessible.Grouping
    Accessible.name: i18n("Audio levels control")
    Accessible.description: i18n("Input and output gain controls with level meters")

    component GainRow: RowLayout {

        property alias label: gain.label
        property alias unit: gain.unit
        property alias from: gain.from
        property alias to: gain.to
        property alias value: gain.value
        property alias leftLevel: left.value
        property alias rightLevel: right.value

        readonly property real radius: 2.5 * Kirigami.Units.gridUnit

        signal gainChanged(real v)

        EeSpinBox {
            id: gain

            horizontalPadding: 0
            verticalPadding: 0
            labelFillWidth: true
            labelAbove: true
            spinboxLayoutFillWidth: true
            decimals: 2
            stepSize: 0.1
            boxWidth: 5 * Kirigami.Units.gridUnit
            onValueModified: v => {
                parent.gainChanged(v);
            }
        }

        EeAudioLevel {
            id: left

            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            implicitWidth: parent.radius
            implicitHeight: parent.radius
            from: Common.minimumDecibelLevel
            to: 0
            value: 0
            decimals: 0
            convertDecibelToLinear: true
        }

        EeAudioLevel {
            id: right

            Layout.alignment: Qt.AlignRight | Qt.AlignBottom
            implicitWidth: parent.radius
            implicitHeight: parent.radius
            from: Common.minimumDecibelLevel
            to: 0
            value: 0
            decimals: 0
            convertDecibelToLinear: true
        }
    }

    ColumnLayout {
        id: column

        anchors.fill: parent

        Kirigami.CardsLayout {
            id: grid

            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.leftMargin: 3 * Kirigami.Units.mediumSpacing
            Layout.rightMargin: 3 * Kirigami.Units.mediumSpacing
            Layout.fillWidth: true

            GainRow {
                id: inputRow

                label: i18n("Input")
                from: pluginDB.getMinValue("inputGain")
                to: pluginDB.getMaxValue("inputGain")
                value: pluginDB.inputGain
                unit: "dB"
                onGainChanged: v => {
                    pluginDB.inputGain = v;
                }
            }

            GainRow {
                id: outputRow

                label: i18n("Output")
                from: pluginDB.getMinValue("outputGain")
                to: pluginDB.getMaxValue("outputGain")
                value: pluginDB.outputGain
                unit: "dB"
                onGainChanged: v => {
                    pluginDB.outputGain = v;
                }
            }
        }
    }
}
