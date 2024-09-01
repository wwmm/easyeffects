import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

//The ItemDelegate must be inside a Item for Kirigami.ListItemDragHandle to work.
//The item beind dragged can not be the direct child.
Item {
    id: delegateItem

    required property int index
    required property bool bypass
    required property string name
    required property string translatedName

    width: {
        if (parent)
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
            delegateItem.ListView.view.currentIndex = index;
            showPassiveNotification("Clicked on plugin: " + name);
        }

        contentItem: GridLayout {
            Layout.fillWidth: true
            columns: 4
            rows: 1
            columnSpacing: Kirigami.Units.smallSpacing

            Kirigami.Icon {
                source: bypass === true ? "media-playback-pause-symbolic" : "format-align-vertical-bottom-symbolic"
                Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                Layout.alignment: Qt.AlignLeft
            }

            Controls.Label {
                Layout.fillWidth: !listItemDelegate.hovered
                text: translatedName
                color: bypass ? Kirigami.Theme.disabledTextColor : Kirigami.Theme.textColor
            }

            Kirigami.ActionToolBar {
                alignment: Qt.AlignRight
                visible: listItemDelegate.hovered
                actions: [
                    Kirigami.Action {
                        text: i18n("Enable this Effect")
                        icon.name: "system-shutdown-symbolic"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        checkable: true
                        checked: !bypass
                        onTriggered: {
                            showPassiveNotification("Enabled:" + name);
                            if (checked !== !bypass)
                                bypass = !checked;

                        }
                    },
                    Kirigami.Action {
                        text: i18n("Remove this effect")
                        icon.name: "delete"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        onTriggered: {
                            pluginsListModel.remove(index, 1);
                            let indexStart = pluginsListModel.index(0, 0);
                            let indexEnd = pluginsListModel.index(pluginsListModel.count - 1, 0);
                            pluginsListModel.dataChanged(indexStart, indexEnd, []);
                        }
                    }
                ]
            }

            Kirigami.ListItemDragHandle {
                listItem: listItemDelegate
                listView: pluginsListView
                onMoveRequested: (oldIndex, newIndex) => {
                    let indexStart = pluginsListModel.index(0, 0);
                    let indexEnd = pluginsListModel.index(pluginsListModel.count - 1, 0);
                    pluginsListModel.move(oldIndex, newIndex, 1);
                    pluginsListModel.dataChanged(indexStart, indexEnd, []);
                }
            }

        }

    }

}
