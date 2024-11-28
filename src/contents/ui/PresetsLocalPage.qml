import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

Kirigami.Page {
    property var sortedListModel: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

    ColumnLayout {
        anchors.fill: parent

        Kirigami.SearchField {
            id: search

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
            model: sortedListModel

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty")
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
                                    if (FGPresetsBackend.savePreset(presetName))
                                        showPresetsMenuStatus(i18n("Settings Saved to: " + presetName));
                                    else
                                        showPresetsMenuStatus(i18n("Failed to Save Settings to: " + presetName));
                                }
                            },
                            Kirigami.Action {
                                text: i18n("Delete this Preset")
                                icon.name: "delete"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    if (FGPresetsBackend.removePreset(presetName))
                                        showPresetsMenuStatus(i18n("The Preset " + presetName + " Has Been Removed"));
                                    else
                                        showPresetsMenuStatus(i18n("The Preset " + presetName + " Coult Not Be Removed"));
                                }
                            }
                        ]
                    }

                }

            }

        }

    }

}
