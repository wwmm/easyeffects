pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var streamDB

    title: i18n("Effects Blocklist") // qmllint disable
    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: 0
    implicitHeight: appWindow.maxOverlayHeight // qmllint disable

    ColumnLayout {
        id: columnLayout

        Layout.preferredWidth: Kirigami.Units.gridUnit * 30

        Kirigami.ActionTextField {
            id: newBlockedApp

            Layout.fillWidth: true
            placeholderText: i18n("Application Node Name") // qmllint disable
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
                    text: i18n("Add to Excluded Applications") // qmllint disable
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
                source: "bookmarks-symbolic"
            }

            validator: RegularExpressionValidator {
                regularExpression: /^[^\\/]{1,100}$/ //strings without `/` or `\` (max 100 chars)
            }
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
                    text: i18n("Empty") // qmllint disable
                }

                delegate: Controls.ItemDelegate {
                    id: listItemDelegate

                    required property int index

                    readonly property string name: {
                        if (control.streamDB.blocklist.length > 0)
                            return control.streamDB.blocklist[index];

                        return "";
                    }

                    hoverEnabled: true
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
                        Layout.maximumWidth: listView.width

                        Controls.Label {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
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
                                    text: i18n("Remove This App") // qmllint disable
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

    footer: Kirigami.ActionToolBar {
        alignment: Qt.AlignRight
        position: Controls.ToolBar.Footer
        actions: [
            Kirigami.Action {
                text: i18n("Show Excluded Apps") // qmllint disable
                tooltip: i18n("Show excluded applications in the list of players/recorders")
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
