import "Common.js" as Common
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import ee.database as DB

FormCard.AbstractFormDelegate {
    id: control

    property real value: 0
    property real from: 0
    property real to: 1
    property int decimals: 2
    property string label: ""
    property string unit: ""
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap
    property bool rightToLeft: false
    property bool convertDecibelToLinear: false
    readonly property real dbFrom: Common.dbToLinear(from)
    readonly property real dbTo: Common.dbToLinear(to)
    readonly property real clampedValue: Common.clamp(value, from, to)
    readonly property real displayValue: control.rightToLeft === false ? (control.clampedValue > sampleTimer.value ? control.clampedValue : sampleTimer.value) : (control.clampedValue < sampleTimer.value ? control.clampedValue : sampleTimer.value)

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    background: Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.neutralBackgroundColor
        radius: 2
    }

    contentItem: Item {
        id: item

        anchors.fill: parent

        Rectangle {
            id: levelRect

            width: parent.width
            height: parent.height
            color: Kirigami.Theme.alternateBackgroundColor

            transform: Scale {
                xScale: if (control.convertDecibelToLinear) {
                    control.rightToLeft === false ? (Common.dbToLinear(control.clampedValue) - control.dbFrom) / (control.dbTo - control.dbFrom) : (Common.dbToLinear(control.clampedValue) - control.dbTo) / (control.dbFrom - control.dbTo);
                } else {
                    control.rightToLeft === false ? (control.clampedValue - control.from) / (control.to - control.from) : (control.clampedValue - control.to) / (control.from - control.to);
                }
                origin.x: control.rightToLeft === false ? 0 : levelRect.width
            }
        }

        Rectangle {
            id: histRect

            width: Kirigami.Units.smallSpacing
            height: parent.height
            color: Kirigami.Theme.positiveTextColor

            transform: Translate {
                x: if (control.convertDecibelToLinear) {
                    control.rightToLeft === false ? (Common.dbToLinear(control.displayValue) - control.dbFrom) / (control.dbTo - control.dbFrom) * item.width : item.width - (Common.dbToLinear(control.displayValue) - control.dbTo) / (control.dbFrom - control.dbTo) * item.width;
                } else {
                    control.rightToLeft === false ? (control.displayValue - control.from) / (control.to - control.from) * item.width : item.width - (control.displayValue - control.to) / (control.from - control.to) * item.width;
                }
            }
        }

        RowLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.smallSpacing
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                text: control.label
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }

            Label {
                Layout.rightMargin: Kirigami.Units.smallSpacing
                horizontalAlignment: Qt.AlignRight
                verticalAlignment: Qt.AlignVCenter
                text: Number(control.displayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + " " + control.unit
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 1
            }
        }
    }

    Timer {
        id: sampleTimer

        property real value: control.clampedValue

        interval: DB.Manager.main.levelMetersLabelTimer
        repeat: true
        running: control.visible

        onTriggered: {
            value = control.clampedValue;
        }
    }
}
