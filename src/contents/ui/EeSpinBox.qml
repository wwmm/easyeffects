/**
 * It is really bizarre that QML does not have a dedicated double spinbox...
 * It is necessary to do a hack solution based on the int spinbox...
 * Our custom spinbox code was inspired by:
 *
 * https://doc.qt.io/Qt-6/qml-qtquick-controls-spinbox.html
 * https://github.com/LeonnardoVerol/example-double-spinbox-component-qt-qml/blob/main/src/ui/DoubleSpinBox.qml
 * https://github.com/Zren/plasma-applet-lib/blob/master/package/contents/ui/libconfig/SpinBox.qml
 * https://github.com/KDE/kirigami-addons/blob/master/src/formcard/FormSpinBoxDelegate.qml
 * https://bugreports.qt.io/browse/QTBUG-67349
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.AbstractFormDelegate {
    id: control

    property int decimals: 2
    property real value: 0
    property real from: 0
    property real to: 100
    property real stepSize: 1
    property real pageSteps: 10
    property bool editable: true
    property string label: ""
    property string subtitle: ""
    property string unit: ""
    property alias displayText: spinbox.displayText
    property int boxWidth: 10 * Kirigami.Units.gridUnit
    property bool labelAbove: false
    property bool labelFillWidth: true
    property bool spinboxLayoutFillWidth: false
    property real spinboxMaximumWidth: -1
    property real spinboxMinimumWidth: -1
    property int elide: Text.ElideRight
    property int wrapMode: Text.Wrap
    property int spinboxAlignment: Qt.AlignRight
    property int labelAlignment: Qt.AlignLeft
    property bool minusInfinityMode: false
    property int maximumLineCount: 2 // -1 to unset

    signal valueModified(real value)

    Accessible.name: control.label
    Accessible.description: `Value: ${control.value} ${control.unit ? ' ' + control.unit : ''}`
    Accessible.role: Accessible.SpinBox

    background: null

    verticalPadding: Kirigami.Units.largeSpacing

    focusPolicy: Kirigami.Settings.isMobile ? Qt.StrongFocus : Qt.NoFocus
    onClicked: spinbox.forceActiveFocus()

    Keys.onPressed: event => {
        if (event.key === Qt.Key_PageUp) {
            const v = control.value + pageSteps * stepSize;
            control.valueModified(Common.clamp(v, control.from, control.to));
            event.accepted = true;
        } else if (event.key === Qt.Key_PageDown) {
            const v = control.value - pageSteps * stepSize;
            control.valueModified(Common.clamp(v, control.from, control.to));
            event.accepted = true;
        }
    }

    contentItem: GridLayout {
        columns: control.labelAbove === false ? 2 : 1
        rows: control.labelAbove === false ? 1 : 2
        rowSpacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            id: labelColumn

            Label {
                id: label

                Layout.fillWidth: control.labelFillWidth
                text: control.label
                elide: control.elide
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                maximumLineCount: control.maximumLineCount > 0 ? control.maximumLineCount : Number.POSITIVE_INFINITY
                horizontalAlignment: control.labelAlignment
            }

            Label {
                id: subtitle

                Layout.fillWidth: control.labelFillWidth
                text: control.subtitle
                elide: control.elide
                color: Kirigami.Theme.disabledTextColor
                wrapMode: control.wrapMode
                horizontalAlignment: control.labelAlignment
                visible: !Common.isEmpty(control.subtitle)
            }
        }

        SpinBox {
            id: spinbox

            readonly property real decimalFactor: Math.pow(10, control.decimals)

            function decimalToInt(decimal) {
                return Math.round(decimal * decimalFactor);
            }

            Layout.fillWidth: control.spinboxLayoutFillWidth
            Layout.maximumWidth: control.spinboxMaximumWidth
            Layout.minimumWidth: control.spinboxMinimumWidth
            Layout.alignment: control.spinboxAlignment
            implicitWidth: control.boxWidth
            focusPolicy: control.focusPolicy
            wheelEnabled: true
            onValueModified: {
                const newValue = Math.round((spinbox.value / spinbox.decimalFactor) * decimalFactor) / decimalFactor;

                control.valueModified(Common.clamp(newValue, control.from, control.to));
            }
            stepSize: spinbox.decimalToInt(control.stepSize)
            value: spinbox.decimalToInt(control.value)
            to: spinbox.decimalToInt(control.to)
            from: spinbox.decimalToInt(control.from)
            editable: control.editable
            inputMethodHints: Qt.ImhPreferNumbers
            textFromValue: (value, locale) => {
                const locMinInfinity = i18nc("minus infinity abbreviation", "-inf");
                const unit_str = (Common.isEmpty(control.unit)) ? "" : ` ${control.unit}`;
                locale.numberOptions = Locale.OmitGroupSeparator;
                const decimalValue = value / spinbox.decimalFactor;

                // Lower bound check in minusInfinityMode.
                if (control.minusInfinityMode === true && decimalValue <= control.from) {
                    textInputSpinBox.text = locMinInfinity;
                    return locMinInfinity;
                }

                // Locale text conversion.
                try {
                    const t = Number(decimalValue).toLocaleString(locale, 'f', control.decimals) + unit_str;
                    textInputSpinBox.text = t;
                    return t;
                } catch (e) {
                    console.warn("Spinbox locale text to number conversion failed:", value);
                    console.warn(e?.message ?? "");
                    textInputSpinBox.text = "";
                    return "";
                }
            }
            valueFromText: (inputText, locale) => {
                /**
                 * We used to have a RegularExpressionValidator to validate
                 * numbers by QML before invoking this function, but we had
                 * to remove it to handle the locale -Infinity string
                 * abbreviation.
                 * So we accept all types of strings and check for the -inf
                 * lower bound. If the check fails, then we apply the regular
                 * expression inside this function.
                 */
                const text = inputText.trim();
                const locMinInfinity = i18nc("minus infinity abbreviation", "-inf");

                if (text === "") {
                    return spinbox.value;
                }

                /**
                 * -Infinity check.
                 * Here we don't have to localize the minus infinity symbol -∞
                 * since we only handle the special case when the user inputs it
                 * manually in the user interface field.
                 */
                if (text.toLowerCase() === locMinInfinity.toLowerCase() || text === "-∞") {
                    return Math.floor(control.from * spinbox.decimalFactor);
                }

                /**
                 * Number validation.
                 * We need also `−` (U+2212) minus sign for Suomi localization.
                 * Some other localizations may also use `＋` (U+FF0B) plus
                 * sign, so let's allow it too for a more robust input
                 * handling. See #4417.
                 */
                const numValidator = /^\s*[-−+＋]?(?:\d+(?:[.,]\d*)?(?:[eE][-−+＋]?\d+)?)/;
                if (text.match(numValidator) === null) {
                    console.warn("Spinbox number validation failed:", text);
                    return spinbox.value;
                }

                // Handling scientific notation.
                const cleanedText = text.replace(/[^\d.,eE+＋-−]/g, '');
                try {
                    const n = Number.fromLocaleString(locale, cleanedText);
                    return !Number.isNaN(n) ? Math.round(n * spinbox.decimalFactor) : spinbox.value;
                } catch (e) {
                    console.warn("Spinbox text to locale number conversion failed:", text);
                    console.warn(e?.message ?? "");
                    return spinbox.value;
                }
            }

            contentItem: TextInput {
                id: textInputSpinBox

                z: 2
                font: spinbox.font
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                selectionColor: Kirigami.Theme.highlightColor
                readOnly: !spinbox.editable
                inputMethodHints: Qt.ImhPreferNumbers
                clip: true
            }
        }
    }
}
