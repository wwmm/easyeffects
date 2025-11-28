import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Control {
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

    readonly property real normalizedClampedValue: (control.clampedValue - control.from) / (control.to - control.from)
    readonly property real normalizedClampedValueDB: (Common.dbToLinear(control.clampedValue) - control.dbFrom) / (control.dbTo - control.dbFrom)

    readonly property real normalizedDisplayValue: (control.displayValue - control.from) / (control.to - control.from)
    readonly property real normalizedDisplayValueDB: (Common.dbToLinear(control.displayValue) - control.dbFrom) / (control.dbTo - control.dbFrom)

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    background: null

    Layout.fillWidth: true

    contentItem: Rectangle {
        id: item

        color: Kirigami.Theme.neutralBackgroundColor
        radius: 0

        implicitHeight: Kirigami.Units.largeSpacing * 3

        Rectangle {
            id: levelRect

            width: parent.width
            height: parent.height
            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.3)

            transform: Scale {
                id: levelScale

                xScale: if (control.convertDecibelToLinear) {
                    control.rightToLeft === false ? control.normalizedClampedValueDB : (Common.dbToLinear(control.clampedValue) - control.dbTo) / (control.dbFrom - control.dbTo);
                } else {
                    control.rightToLeft === false ? control.normalizedClampedValue : (control.clampedValue - control.to) / (control.from - control.to);
                }
                origin.x: control.rightToLeft === false ? 0 : levelRect.width

                Behavior on xScale {
                    NumberAnimation {
                        duration: 100
                        easing.type: Easing.OutQuad
                    }
                }
            }
        }

        Rectangle {
            id: histRect

            width: Kirigami.Units.smallSpacing * 0.5
            height: parent.height
            radius: Kirigami.Units.cornerRadius
            color: levelScale.xScale < 0.85 ? Kirigami.Theme.neutralTextColor : Kirigami.Theme.negativeTextColor

            transform: Translate {
                x: if (control.convertDecibelToLinear) {
                    control.rightToLeft === false ? control.normalizedDisplayValueDB * item.width : item.width - (Common.dbToLinear(control.displayValue) - control.dbTo) / (control.dbFrom - control.dbTo) * item.width;
                } else {
                    control.rightToLeft === false ? control.normalizedDisplayValue * item.width : item.width - (control.displayValue - control.to) / (control.from - control.to) * item.width;
                }

                Behavior on x {
                    NumberAnimation {
                        duration: 400
                        easing.type: Easing.OutCubic
                    }
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
                text: Number(control.displayValue).toLocaleString(Qt.locale(), 'f', control.decimals) + ` ${control.unit}`
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

        interval: DbMain.levelMetersLabelTimer
        repeat: true
        running: control.visible

        onTriggered: {
            value = control.clampedValue;
        }
    }
}
