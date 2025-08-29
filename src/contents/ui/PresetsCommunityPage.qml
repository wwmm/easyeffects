import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: columnLayout

    readonly property int pipeline: {
        if (DB.Manager.main.visiblePage === 0)
            return 1;
        else if (DB.Manager.main.visiblePage === 1)
            return 0;
    }
    readonly property string lastLoadedPresetName: {
        if (DB.Manager.main.visiblePage === 0)
            return DB.Manager.main.lastLoadedOutputPreset;
        else if (DB.Manager.main.visiblePage === 1)
            return DB.Manager.main.lastLoadedInputPreset;
        return "";
    }
    readonly property string lastLoadedCommunityPackage: {
        if (DB.Manager.main.visiblePage === 0)
            return DB.Manager.main.lastLoadedOutputCommunityPackage;
        else if (DB.Manager.main.visiblePage === 1)
            return DB.Manager.main.lastLoadedInputCommunityPackage;
        return "";
    }

    function showPresetsMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }
    function showPresetsMenuError(label) {
        status.text = label;
        status.visible = true;
        status.type = Kirigami.MessageType.Error;
    }

    ListModel {
        id: listModel
    }

    Kirigami.SearchField {
        id: search

        readonly property var sortedListModel: DB.Manager.main.visiblePage === 0 ? Presets.SortedCommunityOutputListModel : Presets.SortedCommunityInputListModel

        Layout.fillWidth: true
        placeholderText: i18n("Search")
        onAccepted: {
            const re = Common.regExpEscape(search.text);
            sortedListModel.filterRegularExpression = RegExp(re, "i");
        }
    }

    RowLayout {
        id: listviewRow

        ListView {
            id: listView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            reuseItems: true
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedCommunityOutputListModel : Presets.SortedCommunityInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty")
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name
                required property string path
                required property string presetPackage

                hoverEnabled: true
                width: listView.width
                onClicked: {
                    if (Presets.Manager.loadCommunityPresetFile(pipeline, path, presetPackage) === false)
                        showPresetsMenuError(i18n("The Preset %1 Failed to Load", `<strong>${name}</strong>`));
                }

                contentItem: RowLayout {
                    Controls.Label {
                        text: name
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: presetPackage
                                displayHint: Kirigami.DisplayHint.KeepVisible
                                icon.name: "package-symbolic"
                                enabled: false
                            },
                            Kirigami.Action {
                                text: i18n("Copy to the Local List")
                                icon.name: "document-import-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    if (Presets.Manager.importFromCommunityPackage(pipeline, path, presetPackage) === true)
                                        showPresetsMenuStatus(i18n("Imported the Community Preset") + ": " + `<strong>${name}</strong>`);
                                    else
                                        showPresetsMenuError(i18n("Failed to Import the Community Preset") + ": " + `<strong>${name}</strong>`);
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

    Controls.Button {
        Layout.alignment: Qt.AlignCenter
        text: i18n("Refresh")
        onClicked: {
            Presets.Manager.refreshCommunityPresets(pipeline);
        }
    }

    Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        visible: false
        showCloseButton: true
    }
}
