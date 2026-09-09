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
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

//The ItemDelegate must be inside an Item for Kirigami.ListItemDragHandle to work.
//The item being dragged cannot be the direct child.
Item {
    id: delegateItem

    required property var listModel
    required property var listView
    required property int index
    required property string name
    required property string translatedName
    required property var pluginDB
    required property var streamDB

    readonly property bool bypass: delegateItem.pluginDB?.bypass ?? false

    signal selectedChanged(string name)
    signal duplicate(string oldName, string newName)

    width: ListView.view.width

    height: listItemDelegate.height

    Delegates.RoundedItemDelegate {
        id: listItemDelegate

        anchors {
            left: parent.left
            right: parent.right
        }

        hoverEnabled: true
        highlighted: delegateItem.ListView.isCurrentItem
        onClicked: {
            delegateItem.ListView.view.currentIndex = delegateItem.index;

            delegateItem.selectedChanged(delegateItem.name);
        }

        contentItem: GridLayout {
            id: pluginRowItem

            columns: 4
            rows: 1
            columnSpacing: Kirigami.Units.smallSpacing

            function toggledEffect(checked) {
                if (checked === !delegateItem.bypass) {
                    return;
                }

                delegateItem.pluginDB.bypass = !checked;
            }

            function duplicateEffect() {
                let plugins = delegateItem.streamDB.plugins.slice();

                let index_list = [];
                let baseName = delegateItem.name.replace(/#\d+$/, "");

                // get a list of every plugin index # 
                for (let n = 0; n < plugins.length; n++) {
                    if (plugins[n].startsWith(baseName)) {
                        const m = plugins[n].match(/#(\d+)$/);
                        if (m.length === 2) {
                            index_list.push(m[1]);
                        }
                    }
                }

                // base the new plugin off the highest index #
                const new_id = (index_list.length === 0) ? null : Math.max.apply(null, index_list) + 1;
                
                // create the new plugin name (used as a unique identifier)
                const newName = baseName + "#" + new_id;

                // since we are duplicating, new_id should NEVER be null
                if (new_id != null) {
                    // add to list
                    plugins.splice(delegateItem.index + 1, 0, newName);

                    // save list
                    delegateItem.streamDB.plugins = plugins;

                    // select new plugin
                    delegateItem.streamDB.visiblePlugin = newName;

                    // update new plugin with original plugin's data
                    delegateItem.duplicate(delegateItem.name, newName);

                    appWindow.showStatus(i18n("Added a new effect to the pipeline: %1", `<strong>${delegateItem.translatedName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                }
            }

            function removedEffect() {
                /**
                 * If the selected item is removed we set the one before
                 * it as the visible plugin.
                 */

                if (delegateItem.listModel.count > 1 && delegateItem.index > 0 && delegateItem.ListView.view.currentIndex === delegateItem.index) {
                    delegateItem.streamDB.visiblePlugin = delegateItem.listModel.get(delegateItem.index - 1).name;
                }

                delegateItem.listModel.remove(delegateItem.index, 1);

                const indexStart = delegateItem.listModel.index(0, 0);
                const indexEnd = delegateItem.listModel.index(delegateItem.listModel.count - 1, 0);

                delegateItem.listModel.dataChanged(indexStart, indexEnd, []);
            }

            Kirigami.Icon {
                source: bypass === true ? "media-playback-pause-symbolic" : "composition-symbolic"
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.alignment: Qt.AlignLeft
            }

            Controls.Label {
                Layout.fillWidth: true
                text: DbMain.collapsePluginsList === false ? delegateItem.translatedName : delegateItem.translatedName.charAt(0)
                color: bypass ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
                elide: Text.ElideRight
            }

            Kirigami.ActionToolBar {
                id: pluginActionButtonControls

                alignment: Qt.AlignRight
                visible: !DbMain.collapsePluginsList
                Layout.preferredWidth: !DbMain.reducePluginsListControls ? maximumContentWidth + Kirigami.Units.smallSpacing : -1
                actions: [
                    Kirigami.Action {
                        text: i18n("Toggle this effect") // qmllint disable
                        icon.name: "system-shutdown-symbolic"
                        displayHint: DbMain.reducePluginsListControls ? Kirigami.DisplayHint.AlwaysHide : (Kirigami.DisplayHint.IconOnly | Kirigami.DisplayHint.KeepVisible)
                        checkable: true
                        checked: !bypass
                        onTriggered: pluginRowItem.toggledEffect(checked)
                    },
                    Kirigami.Action {
                        text: i18n("Duplicate this effect") // qmllint disable
                        icon.name: "document-duplicate"
                        displayHint: DbMain.reducePluginsListControls ? Kirigami.DisplayHint.AlwaysHide : (Kirigami.DisplayHint.IconOnly | Kirigami.DisplayHint.KeepVisible)
                        onTriggered: pluginRowItem.duplicateEffect()
                    },
                    Kirigami.Action {
                        text: i18n("Remove this effect") // qmllint disable
                        icon.name: "delete"
                        displayHint: DbMain.reducePluginsListControls ? Kirigami.DisplayHint.AlwaysHide : (Kirigami.DisplayHint.IconOnly | Kirigami.DisplayHint.KeepVisible)
                        onTriggered: pluginRowItem.removedEffect()
                    }
                ]
            }

            Kirigami.ListItemDragHandle {
                listItem: listItemDelegate
                listView: delegateItem.listView
                visible: !DbMain.collapsePluginsList
                onMoveRequested: (oldIndex, newIndex) => {
                    if (oldIndex === newIndex)
                        return;

                    delegateItem.listModel.move(oldIndex, newIndex, 1);
                }
                onDragActiveChanged: {
                    if (dragActive === false) {
                        const indexStart = delegateItem.listModel.index(0, 0);
                        const indexEnd = delegateItem.listModel.index(delegateItem.listModel.count - 1, 0);

                        delegateItem.listModel.dataChanged(indexStart, indexEnd, []);
                    }
                }
            }
        }
    }
}
