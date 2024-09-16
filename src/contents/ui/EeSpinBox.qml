/*
  It is really bizarre that qml does not have a dedicated double spinbox... It is necessary to do a hack
  solution based on the int spinbox... Our custom spinbox code was inspired by:

https://doc.qt.io/Qt-6/qml-qtquick-controls-spinbox.html
https://github.com/LeonnardoVerol/example-double-spinbox-component-qt-qml/blob/main/src/ui/DoubleSpinBox.qml
https://github.com/Zren/plasma-applet-lib/blob/master/package/contents/ui/libconfig/SpinBox.qml
https://github.com/KDE/kirigami-addons/blob/master/src/formcard/FormSpinBoxDelegate.qml
https://bugreports.qt.io/browse/QTBUG-67349

 */

import "Common.js" as Common
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: control

    property int decimals: 2
    property real value: 0
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property bool editable: true
    property string label: ""
    property string subtitle: ""
    property string unit: ""
    property alias displayText: spinbox.displayText
    property var status: Kirigami.MessageType.Information
    property string statusMessage: ""
    property int boxWidth: 10 * Kirigami.Units.gridUnit
    property bool labelAbove: false
    property bool spinboxLayoutFillWidth: false
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap

    signal valueModified(real value)

    focusPolicy: Kirigami.Settings.isMobile ? Qt.StrongFocus : Qt.NoFocus
    background: null
    onClicked: spinbox.forceActiveFocus()

    contentItem: GridLayout {
        Layout.fillWidth: true
        columns: labelAbove === false ? 2 : 1
        rows: labelAbove === false ? 1 : 2
        rowSpacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            Label {
                Layout.fillWidth: true
                text: control.label
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 2
            }

            Label {
                Layout.fillWidth: true
                text: control.subtitle
                elide: control.elide
                color: Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: 2
                visible: !Common.isEmpty(control.subtitle)
            }

        }

        SpinBox {
            id: spinbox

            readonly property real decimalFactor: Math.pow(10, control.decimals)

            function decimalToInt(decimal) {
                return decimal * decimalFactor;
            }

            Layout.fillWidth: control.spinboxLayoutFillWidth
            implicitWidth: control.boxWidth
            focusPolicy: control.focusPolicy
            wheelEnabled: true
            onValueModified: {
                control.valueModified(spinbox.value * 1 / spinbox.decimalFactor);
            }
            stepSize: spinbox.decimalToInt(control.stepSize)
            value: spinbox.decimalToInt(control.value)
            to: spinbox.decimalToInt(control.to)
            from: spinbox.decimalToInt(control.from)
            editable: control.editable
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            textFromValue: (value, locale) => {
                let unit_str = (Common.isEmpty(unit)) ? "" : " " + unit;
                locale.numberOptions = Locale.OmitGroupSeparator;
                let t = Number(value / spinbox.decimalFactor).toLocaleString(locale, 'f', control.decimals) + unit_str;
                textInputSpinBox.text = t;
                return t;
            }
            valueFromText: (text, locale) => {
                let re = /-?\d*[.,]?\d*/;
                let regex_result = re.exec(text);
                let v = Number.fromLocaleString(locale, regex_result[0]) * spinbox.decimalFactor;
                v = (!isNaN(v)) ? Math.round(v) : spinbox.value;
                return v;
            }

            contentItem: TextInput {
                id: textInputSpinBox

                z: 2
                font: spinbox.font
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                selectionColor: Kirigami.Theme.highlightColor
                readOnly: !spinbox.editable
                validator: spinbox.validator
                inputMethodHints: Qt.ImhFormattedNumbersOnly
            }

            validator: DoubleValidator {
                locale: control.locale.name
                notation: DoubleValidator.StandardNotation
                decimals: control.decimals
                bottom: Math.min(spinbox.from, spinbox.to) * spinbox.decimalFactor
                top: Math.max(spinbox.from, spinbox.to) * spinbox.decimalFactor
            }

        }

    }

}
