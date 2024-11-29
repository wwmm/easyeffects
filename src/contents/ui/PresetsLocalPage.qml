import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: columnLayout

    function showPresetsMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

    Kirigami.ActionTextField {
        id: newPresetName

        Layout.fillWidth: true
        placeholderText: i18n("New Preset Name")
        // based on https://github.com/KDE/kirigami/blob/master/src/controls/SearchField.qml
        leftPadding: {
            if (effectiveHorizontalAlignment === TextInput.AlignRight)
                return _rightActionsRow.width + Kirigami.Units.smallSpacing;
            else
                return presetCreationIcon.width + Kirigami.Units.smallSpacing * 3;
        }
        rightPadding: {
            if (effectiveHorizontalAlignment === TextInput.AlignRight)
                return presetCreationIcon.width + Kirigami.Units.smallSpacing * 3;
            else
                return _rightActionsRow.width + Kirigami.Units.smallSpacing;
        }
        rightActions: [
            Kirigami.Action {
                text: i18n("Import Preset File")
                icon.name: "document-import-symbolic"
                onTriggered: {
                    newPresetName.text = "";
                    newPresetName.accepted();
                    fileDialog.open();
                }
            },
            Kirigami.Action {
                text: i18n("Create Preset")
                icon.name: "list-add-symbolic"
                onTriggered: {
                    if (!Common.isEmpty(newPresetName.text)) {
                        const pipeline = DB.Manager.main.visiblePage === 0 ? 1 : 0;
                        if (Presets.Manager.add(pipeline, newPresetName.text) === true) {
                            newPresetName.accepted();
                            showPresetsMenuStatus(i18n("New Preset Created: " + newPresetName.text));
                            newPresetName.text = "";
                        } else {
                            showPresetsMenuStatus(i18n("Failed to Create Preset: " + newPresetName.text));
                        }
                    }
                }
            }
        ]

        Kirigami.Icon {
            id: presetCreationIcon

            LayoutMirroring.enabled: newPresetName.effectiveHorizontalAlignment === TextInput.AlignRight
            anchors.left: newPresetName.left
            anchors.leftMargin: Kirigami.Units.smallSpacing * 2
            anchors.verticalCenter: newPresetName.verticalCenter
            anchors.verticalCenterOffset: Math.round((newPresetName.topPadding - newPresetName.bottomPadding) / 2)
            implicitHeight: Kirigami.Units.iconSizes.sizeForLabels
            implicitWidth: Kirigami.Units.iconSizes.sizeForLabels
            color: newPresetName.placeholderTextColor
            source: "bookmarks-symbolic"
        }

        validator: RegularExpressionValidator {
            regularExpression: /[^\\/]{100}$/ //less than 100 characters and no / or \
        }

    }

    Kirigami.SearchField {
        id: search

        readonly property var sortedListModel: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

        Layout.fillWidth: true
        placeholderText: i18n("Search")
        onAccepted: {
            sortedListModel.filterRegularExpression = RegExp(search.text, "i");
        }
    }

    ListView {
        id: listView

        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        reuseItems: true
        model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count === 0
            text: i18n("Empty")
        }

        Controls.ScrollBar.vertical: Controls.ScrollBar {
        }

        delegate: Controls.ItemDelegate {
            id: listItemDelegate

            required property string name
            property bool selected: listItemDelegate.highlighted || listItemDelegate.down
            property color color: selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            hoverEnabled: true
            width: listView.width

            contentItem: RowLayout {
                Controls.Label {
                    text: name
                }

                Kirigami.ActionToolBar {
                    alignment: Qt.AlignRight
                    actions: [
                        Kirigami.Action {
                            text: i18n("Save Settings to this Preset")
                            icon.name: "document-save-symbolic"
                            displayHint: Kirigami.DisplayHint.AlwaysHide
                            onTriggered: {
                                if (FGPresetsBackend.savePreset(name))
                                    showPresetsMenuStatus(i18n("Settings Saved to: " + name));
                                else
                                    showPresetsMenuStatus(i18n("Failed to Save Settings to: " + name));
                            }
                        },
                        Kirigami.Action {
                            text: i18n("Delete this Preset")
                            icon.name: "delete"
                            displayHint: Kirigami.DisplayHint.AlwaysHide
                            onTriggered: {
                                const pipeline = DB.Manager.main.visiblePage === 0 ? 1 : 0;
                                if (Presets.Manager.remove(pipeline, name) === true)
                                    showPresetsMenuStatus(i18n("The Preset " + name + " Has Been Removed"));
                                else
                                    showPresetsMenuStatus(i18n("The Preset " + name + " Coult Not Be Removed"));
                            }
                        }
                    ]
                }

            }

        }

    }

    Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        visible: false
        showCloseButton: true
        Layout.maximumWidth: parent.width
    }

}
