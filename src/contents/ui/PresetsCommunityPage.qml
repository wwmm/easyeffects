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

    ListModel {
        id: listModel
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
        // model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel

        id: listView

        Layout.fillWidth: true
        Layout.fillHeight: true
        clip: true
        reuseItems: true

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
            onClicked: {
                if (Presets.Manager.loadLocalPresetFile(pipeline, name) === false)
                    showPresetsMenuStatus(i18n("The Preset " + name + "failed to load"));

            }

            contentItem: RowLayout {
                Controls.Label {
                    text: name
                }

                Kirigami.ActionToolBar {
                    alignment: Qt.AlignRight
                    actions: [
                        Kirigami.Action {
                            // if (Presets.Manager.savePresetFile(pipeline, name) === true)
                            //     showPresetsMenuStatus(i18n("Settings Saved to: " + name));
                            // else
                            //     showPresetsMenuStatus(i18n("Failed to Save Settings to: " + name));

                            text: i18n("Import")
                            icon.name: "document-import-symbolic"
                            displayHint: Kirigami.DisplayHint.KeepVisible
                            onTriggered: {
                            }
                        }
                    ]
                }

            }

        }

    }

    Controls.Button {
        Layout.alignment: Qt.AlignCenter
        text: i18n("Refresh")
        onClicked: showPassiveNotification("refreshing!")
    }

    Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        visible: false
        showCloseButton: true
    }

}
