import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

// Item required to make Kirigami.ListItemDragHandle work. THe item beind dragged can not be the direct child
Item {
    width: parent ? parent.width : listItemDelegate.implicitWidth
    height: listItemDelegate.height

    Controls.ItemDelegate {
        id: listItemDelegate

        property int wrapMode: Text.WrapAnywhere
        property int elide: Text.ElideRight
        property bool selected: listItemDelegate.highlighted || listItemDelegate.down
        property color color: selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

        hoverEnabled: true
        width: parent.width
        onClicked: {
            showPassiveNotification("Clicked on plugin: " + model.name);
        }

        contentItem: RowLayout {
            Kirigami.Icon {
                source: model.stateIconName
                Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                Layout.alignment: Qt.AlignLeft
            }

            Controls.Label {
                Layout.fillWidth: !listItemDelegate.hovered
                text: model.name
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
                        checked: !model.bypass
                        onTriggered: {
                            showPassiveNotification("Enabled:" + model.name);
                            if (checked !== !model.bypass)
                                model.bypass = !checked;

                        }
                    },
                    Kirigami.Action {
                        text: i18n("Remove this effect")
                        icon.name: "delete"
                        displayHint: Kirigami.DisplayHint.IconOnly
                        onTriggered: {
                            showPassiveNotification("Removed plugin: " + model.name);
                        }
                    }
                ]
            }

            Kirigami.ListItemDragHandle {
                listItem: listItemDelegate
                listView: pluginsListView
                onMoveRequested: (oldIndex, newIndex) => {
                    pluginsListModel.move(oldIndex, newIndex, 1);
                }
            }

        }

    }

}
