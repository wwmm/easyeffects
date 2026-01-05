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
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: sofaDialog

    required property var pluginDB
    required property var pluginBackend

    title: i18n("Target Orientation") // qmllint disable
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing

    ColumnLayout {
        enabled: pluginBackend?.kernelIsSofa ?? false
        spacing: Kirigami.Units.gridUnit

        EeSpinBox {
            label: i18n("Azimuth") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: sofaDialog.pluginBackend?.sofaMinAzimuth ?? 0
            to: sofaDialog.pluginBackend?.sofaMaxAzimuth ?? 0
            value: sofaDialog.pluginDB?.targetSofaAzimuth ?? 0
            decimals: 0
            stepSize: 1
            unit: Units.degrees
            separateUnit: false
            onValueModified: v => {
                sofaDialog.pluginDB.targetSofaAzimuth = v;
            }
        }

        EeSpinBox {
            label: i18n("Elevation") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: sofaDialog.pluginBackend?.sofaMinElevation ?? 0
            to: sofaDialog.pluginBackend?.sofaMaxElevation ?? 0
            value: sofaDialog.pluginDB?.targetSofaElevation ?? 0
            decimals: 0
            stepSize: 1
            unit: Units.degrees
            separateUnit: false
            onValueModified: v => {
                sofaDialog.pluginDB.targetSofaElevation = v;
            }
        }

        EeSpinBox {
            label: i18n("Radius") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: sofaDialog.pluginBackend?.sofaMinRadius ?? 0
            to: sofaDialog.pluginBackend?.sofaMaxRadius ?? 0
            value: sofaDialog.pluginDB?.targetSofaRadius ?? 0
            decimals: 1
            stepSize: 0.1
            unit: Units.m
            onValueModified: v => {
                sofaDialog.pluginDB.targetSofaRadius = v;
            }
        }

        Controls.Label {
            Layout.alignment: Qt.AlignHCenter
            text: i18n("Status") // qmllint disable
        }

        GridLayout {
            id: delegateLayout

            Layout.alignment: Qt.AlignHCenter
            uniformCellWidths: true
            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            columns: 3
            rows: 4

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Database") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Measurements") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Index") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaDatabase : ""
                enabled: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: Number(sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaMeasurements : 0).toLocaleString(Qt.locale(), 'f', 0)
                enabled: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: Number(sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaIndex : 0).toLocaleString(Qt.locale(), 'f', 0)
                enabled: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Azimuth") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Elevation") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: i18n("Radius") // qmllint disable
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: Number(sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaAzimuth : 0).toLocaleString(Qt.locale(), 'f', 0) + Units.degrees
                enabled: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: Number(sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaElevation : 0).toLocaleString(Qt.locale(), 'f', 0) + Units.degrees
                enabled: false
            }

            Controls.Label {
                Layout.alignment: Qt.AlignHCenter
                text: Number(sofaDialog.pluginBackend ? sofaDialog.pluginBackend.sofaRadius : 0).toLocaleString(Qt.locale(), 'f', 1) + ` ${Units.m}`
                enabled: false
            }
        }
    }

    customFooterActions: Kirigami.Action {
        enabled: pluginBackend?.kernelIsSofa ?? false
        text: i18n("Apply") // qmllint disable
        icon.name: "dialog-ok-apply-symbolic"
        onTriggered: {
            sofaDialog.pluginBackend.applySofaOrientation();
        }
    }
}
