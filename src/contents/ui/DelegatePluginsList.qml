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

    signal selectedChanged(string name)

    width: {
        if (parent && !DB.Manager.main.collapsePluginsList)
            parent.width > listItemDelegate.implicitWidth ? parent.width : listItemDelegate.implicitWidth;
        else
            listItemDelegate.implicitWidth;
    }
    height: listItemDelegate.height

    Controls.ItemDelegate {
        id: listItemDelegate

        hoverEnabled: true
        width: parent.width
        highlighted: delegateItem.ListView.isCurrentItem
        onClicked: {
            delegateItem.ListView.view.currentIndex = delegateItem.index;

            delegateItem.selectedChanged(delegateItem.name);
        }

        contentItem: GridLayout {
            columns: 4
            rows: 1
            columnSpacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: delegateItem.pluginDB.bypass === true ? "media-playback-pause-symbolic" : "composition-symbolic"
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                Layout.alignment: Qt.AlignLeft
            }

            Controls.Label {
                Layout.fillWidth: true
                text: DB.Manager.main.collapsePluginsList === false ? delegateItem.translatedName : delegateItem.translatedName.charAt(0)
                color: delegateItem.pluginDB.bypass ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                visible: !DB.Manager.main.collapsePluginsList
                actions: [
                    Kirigami.Action {
                        text: i18n("Toggle This Effect") // qmllint disable
                        icon.name: "system-shutdown-symbolic"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        checkable: true
                        checked: !delegateItem.pluginDB.bypass
                        onTriggered: {
                            if (checked === !delegateItem.pluginDB.bypass) {
                                return;
                            }

                            delegateItem.pluginDB.bypass = !checked;
                        }
                    },
                    Kirigami.Action {
                        text: i18n("Remove This Effect") // qmllint disable
                        icon.name: "delete"
                        displayHint: Kirigami.DisplayHint.AlwaysHide
                        onTriggered: {
                            /*
                            * If the selected item is removed we set the one before it as the visible plugin
                            */

                            if (delegateItem.listModel.count > 1 && delegateItem.index > 0 && delegateItem.ListView.view.currentIndex === delegateItem.index) {
                                delegateItem.streamDB.visiblePlugin = delegateItem.pluginsListModel.get(delegateItem.index - 1).name;
                            }

                            delegateItem.listModel.remove(delegateItem.index, 1);

                            const indexStart = delegateItem.listModel.index(0, 0);
                            const indexEnd = delegateItem.listModel.index(delegateItem.listModel.count - 1, 0);

                            delegateItem.listModel.dataChanged(indexStart, indexEnd, []);
                        }
                    }
                ]
            }

            Kirigami.ListItemDragHandle {
                listItem: listItemDelegate
                listView: delegateItem.listView
                visible: !DB.Manager.main.collapsePluginsList
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
