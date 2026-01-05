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

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

Kirigami.Dialog {
    id: control

    required property var streamDB

    title: i18n("Excluded Applications") // qmllint disable
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8) // qmllint disable
    implicitHeight: Math.min(Kirigami.Units.gridUnit * 40, Math.round(Controls.ApplicationWindow.window.height * 0.8))
    bottomPadding: 1
    anchors.centerIn: parent

    ColumnLayout {
        id: columnLayout

        spacing: 0

        Kirigami.ActionTextField {
            id: newBlockedApp

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            placeholderText: i18n("Application node name") // qmllint disable
            // based on https://github.com/KDE/kirigami/blob/master/src/controls/SearchField.qml
            leftPadding: {
                if (effectiveHorizontalAlignment === TextInput.AlignRight)
                    return _rightActionsRow.width + Kirigami.Units.smallSpacing;
                else
                    return creationIcon.width + Kirigami.Units.smallSpacing * 3;
            }
            rightPadding: {
                if (effectiveHorizontalAlignment === TextInput.AlignRight)
                    return creationIcon.width + Kirigami.Units.smallSpacing * 3;
                else
                    return _rightActionsRow.width + Kirigami.Units.smallSpacing;
            }
            rightActions: [
                Kirigami.Action {
                    text: i18n("Add to excluded applications") // qmllint disable
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        const name = newBlockedApp.text;
                        // trim to exclude names containing only multiple spaces
                        if (!Common.isEmpty(name.trim())) {
                            if (!control.streamDB.blocklist.includes(name)) {
                                control.streamDB.blocklist.push(name);
                                newBlockedApp.text = "";
                            }
                        }
                    }
                }
            ]

            Kirigami.Icon {
                id: creationIcon

                LayoutMirroring.enabled: newBlockedApp.effectiveHorizontalAlignment === TextInput.AlignRight
                anchors.left: newBlockedApp.left
                anchors.leftMargin: Kirigami.Units.smallSpacing * 2
                anchors.verticalCenter: newBlockedApp.verticalCenter
                anchors.verticalCenterOffset: Math.round((newBlockedApp.topPadding - newBlockedApp.bottomPadding) / 2)
                implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
                implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
                color: newBlockedApp.placeholderTextColor
                source: "application-x-executable"
            }

            validator: Validators.validFileNameRegex
        }

        RowLayout {
            id: listviewRow

            ListView {
                id: listView

                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.minimumHeight: control.parent.height - 2 * (control.header.height + control.footer.height + newBlockedApp.height) - control.y
                clip: true
                reuseItems: true
                model: control.streamDB.blocklist
                Controls.ScrollBar.vertical: listViewScrollBar

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: listView.count === 0
                    text: i18n("Empty List") // qmllint disable
                    explanation: i18n("No application excluded") // qmllint disable
                }

                delegate: Delegates.RoundedItemDelegate {
                    id: listItemDelegate

                    required property int index

                    readonly property string name: {
                        if (control.streamDB.blocklist.length > 0)
                            return control.streamDB.blocklist[index];

                        return "";
                    }

                    hoverEnabled: true
                    highlighted: false
                    down: false
                    width: listView.width

                    Kirigami.PromptDialog {
                        id: deleteDialog

                        title: i18n("Remove Application") // qmllint disable
                        subtitle: i18n("Are you sure you want to remove this application from the list?") // qmllint disable
                        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
                        onAccepted: {
                            const targetIndex = control.streamDB.blocklist.indexOf(listItemDelegate.name);
                            if (targetIndex > -1)
                                control.streamDB.blocklist.splice(targetIndex, 1);
                        }
                    }

                    contentItem: RowLayout {
                        Layout.maximumWidth: listItemDelegate.width

                        Controls.Label {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            Layout.maximumWidth: listItemDelegate.width
                            Layout.horizontalStretchFactor: 1
                            text: listItemDelegate.name
                            elide: Text.ElideRight
                            wrapMode: Text.WrapAnywhere
                            maximumLineCount: 2
                        }

                        Kirigami.ActionToolBar {
                            id: delegateActionToolBar

                            Layout.fillWidth: true
                            Layout.horizontalStretchFactor: 2
                            alignment: Qt.AlignRight
                            actions: [
                                Kirigami.Action {
                                    text: i18n("Remove this app") // qmllint disable
                                    icon.name: "delete"
                                    displayHint: Kirigami.DisplayHint.AlwaysHide
                                    onTriggered: {
                                        deleteDialog.open();
                                    }
                                }
                            ]
                        }
                    }
                }
            }

            Controls.ScrollBar {
                id: listViewScrollBar

                parent: listviewRow
                Layout.fillHeight: true
            }
        }
    }

    footer: RowLayout {
        Kirigami.ActionToolBar {
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            Layout.margins: Kirigami.Units.smallSpacing
            actions: [
                Kirigami.Action {
                    text: i18n("Show excluded apps") // qmllint disable
                    tooltip: i18n("Show excluded applications in the list of players/recorders") // qmllint disable
                    icon.name: "applications-all-symbolic"
                    displayHint: Kirigami.DisplayHint.KeepVisible
                    checkable: true
                    checked: control.streamDB.showBlocklistedApps
                    onTriggered: {
                        if (checked !== control.streamDB.showBlocklistedApps)
                            control.streamDB.showBlocklistedApps = checked;
                    }
                }
            ]
        }
    }
}
