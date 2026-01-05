/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

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
    property bool separateUnit: true
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
                let unitSuffix = "";
                if (!Common.isEmpty(control.unit)) {
                    const split = control.separateUnit ? ' ' : '';
                    unitSuffix = `${split}${control.unit}`;
                }

                // Lower bound check in minusInfinityMode.
                const decimalValue = value / spinbox.decimalFactor;
                if (control.minusInfinityMode === true && decimalValue <= control.from) {
                    textInputSpinBox.text = Units.minInf;
                    return Units.minInf;
                }

                // Locale text conversion.
                try {
                    locale.numberOptions = Locale.OmitGroupSeparator;
                    const t = Number(decimalValue).toLocaleString(locale, 'f', control.decimals) + unitSuffix;
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

                if (text === "") {
                    return spinbox.value;
                }

                /**
                 * -Infinity check.
                 * Here we don't have to localize the minus infinity symbol -∞
                 * since we only handle the special case when the user inputs it
                 * manually in the user interface field.
                 */
                if (text.toLowerCase() === Units.minInf.toLowerCase() || text === "-∞") {
                    return Math.floor(control.from * spinbox.decimalFactor);
                }

                const matchResult = text.match(Validators.spinBoxRegex);

                if (matchResult === null) {
                    console.warn("Spinbox number validation failed:", text);
                    return spinbox.value;
                }

                const num = matchResult[0];

                try {
                    const n = Number.fromLocaleString(locale, num);
                    return !Number.isNaN(n) ? Math.round(n * spinbox.decimalFactor) : spinbox.value;
                } catch (e) {
                    console.warn(`Spinbox locale number conversion failed with text "${text}" and captured number "${num}".`);
                    console.warn(e?.message ?? "");
                    return spinbox.value;
                }
            }

            contentItem: TextInput {
                id: textInputSpinBox

                z: 2
                verticalAlignment: TextInput.AlignVCenter
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
