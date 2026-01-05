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

FormCard.AbstractFormDelegate {
    id: control

    property alias isChecked: qtSwitch.checked
    property string label: ""
    property string subtitle: ""
    property int maximumLineCount: 2 // -1 to unset

    onClicked: qtSwitch.toggle()
    hoverEnabled: true
    focusPolicy: Qt.StrongFocus

    Accessible.role: Accessible.CheckBox
    Accessible.name: control.label
    Accessible.description: control.subtitle
    Accessible.checked: control.checked
    Accessible.onToggleAction: if (control.enabled)
        control.clicked()

    verticalPadding: Kirigami.Units.largeSpacing

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing

            Layout.fillWidth: true

            Label {
                Layout.fillWidth: true
                text: control.label
                elide: Text.ElideRight
                color: control.enabled ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
                wrapMode: Text.Wrap
                maximumLineCount: control.maximumLineCount > 0 ? control.maximumLineCount : Number.POSITIVE_INFINITY
            }

            Label {
                Layout.fillWidth: true
                text: control.subtitle
                elide: Text.ElideRight
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.Wrap
                visible: !Common.isEmpty(control.subtitle)
            }
        }

        Switch {
            id: qtSwitch

            onCheckedChanged: {
                control.checkedChanged();
            }
            Layout.leftMargin: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            enabled: control.enabled
        }
    }
}
