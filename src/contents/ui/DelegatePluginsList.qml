import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

//The ItemDelegate must be inside a Item for Kirigami.ListItemDragHandle to work.
//The item beind dragged can not be the direct child.
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

    width: ListView.view.width

    height: listItemDelegate.height

    Controls.ItemDelegate {
        id: listItemDelegate

        anchors {
            left: parent.left
            right: parent.right
        }

        hoverEnabled: true
        // width: parent.width
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
