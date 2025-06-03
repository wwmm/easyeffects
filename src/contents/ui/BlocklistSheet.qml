import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var streamDB

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    showCloseButton: false
    y: appWindow.header.height + Kirigami.Units.gridUnit
    title: i18n("Effects Blocklist")

    ColumnLayout {
        id: columnLayout

        Layout.preferredWidth: Kirigami.Units.gridUnit * 30

        Kirigami.ActionTextField {
            id: newBlockedApp

            Layout.fillWidth: true
            placeholderText: i18n("Application Node Name")
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
                    text: i18n("Add to Excluded Applications")
                    icon.name: "list-add-symbolic"
                    onTriggered: {
                        const name = newBlockedApp.text;
                        // trim to exclude names containing only multiple spaces
                        if (!Common.isEmpty(name.trim())) {
                            if (!streamDB.blocklist.includes(name)) {
                                streamDB.blocklist.push(name);
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
                model: streamDB.blocklist
                Controls.ScrollBar.vertical: listViewScrollBar

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: listView.count === 0
                    text: i18n("Empty")
                }

                delegate: Controls.ItemDelegate {
                    id: listItemDelegate

                    readonly property string name: {
                        if (streamDB.blocklist.length > 0)
                            return streamDB.blocklist[index];

                        return "";
                    }

                    hoverEnabled: true
                    down: false
                    width: listView.width

                    contentItem: RowLayout {
                        Layout.maximumWidth: listView.width

                        Controls.Label {
                            Layout.alignment: Qt.AlignLeft
                            Layout.fillWidth: true
                            Layout.horizontalStretchFactor: 1
                            text: name
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
                                    text: i18n("Delete this App")
                                    icon.name: "delete"
                                    displayHint: Kirigami.DisplayHint.AlwaysHide
                                    onTriggered: {
                                        const targetIndex = streamDB.blocklist.indexOf(name);
                                        if (targetIndex > -1)
                                            streamDB.blocklist.splice(targetIndex, 1);
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
                text: i18n("Show Excluded Applications in the Streams Section")
                icon.name: "applications-all-symbolic"
                displayHint: Kirigami.DisplayHint.KeepVisible
                checkable: true
                checked: streamDB.showBlocklistedApps
                onTriggered: {
                    if (checked !== streamDB.showBlocklistedApps)
                        streamDB.showBlocklistedApps = checked;
                }
            }
        ]
    }
}
