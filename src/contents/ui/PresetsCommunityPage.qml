pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

ColumnLayout {
    id: columnLayout

    readonly property int pipeline: {
        if (DbMain.visiblePage === 0)
            return 1;
        else if (DbMain.visiblePage === 1)
            return 0;
    }
    readonly property string lastLoadedPresetName: {
        if (DbMain.visiblePage === 0)
            return DbMain.lastLoadedOutputPreset;
        else if (DbMain.visiblePage === 1)
            return DbMain.lastLoadedInputPreset;
        return "";
    }
    readonly property string lastLoadedCommunityPackage: {
        if (DbMain.visiblePage === 0)
            return DbMain.lastLoadedOutputCommunityPackage;
        else if (DbMain.visiblePage === 1)
            return DbMain.lastLoadedInputCommunityPackage;
        return "";
    }

    Kirigami.SearchField {
        id: search

        readonly property var sortedListModel: DbMain.visiblePage === 0 ? Presets.SortedCommunityOutputListModel : Presets.SortedCommunityInputListModel

        Layout.fillWidth: true
        placeholderText: i18n("Search") // qmllint disable
        onAccepted: {
            const re = Common.regExpEscape(search.text);
            sortedListModel.filterRegularExpression = RegExp(re, "i");
        }
        Component.onCompleted: {
            const re = Common.regExpEscape("");
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
            model: DbMain.visiblePage === 0 ? Presets.SortedCommunityOutputListModel : Presets.SortedCommunityInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty List") // qmllint disable
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name
                required property string path
                required property string presetPackage

                hoverEnabled: true
                width: listView.width
                onClicked: Presets.Manager.loadCommunityPresetFile(columnLayout.pipeline, path, presetPackage)

                contentItem: RowLayout {
                    Controls.Label {
                        text: listItemDelegate.name
                        elide: Text.ElideRight
                        wrapMode: Text.WrapAnywhere
                        maximumLineCount: 2
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: listItemDelegate.presetPackage
                                displayHint: Kirigami.DisplayHint.KeepVisible
                                icon.name: "package-symbolic"
                                enabled: false
                            },
                            Kirigami.Action {
                                text: i18n("Copy to local presets") // qmllint disable
                                icon.name: "document-import-symbolic"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    if (Presets.Manager.importFromCommunityPackage(columnLayout.pipeline, listItemDelegate.path, listItemDelegate.presetPackage) === true) {
                                        appWindow.showStatus(i18n("Imported the %1 community preset to the list of local presets.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                                    } else {
                                        appWindow.showStatus(i18n("Failed to import the %1 community preset to the list of local presets.", `<strong>${listItemDelegate.name}</strong>`), Kirigami.MessageType.Error, false); // qmllint disable
                                    }
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
        text: i18n("Refresh") // qmllint disable
        onClicked: {
            Presets.Manager.refreshCommunityPresets(columnLayout.pipeline);
        }
    }
}
