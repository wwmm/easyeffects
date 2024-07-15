import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: listItemDelegate

    property int wrapMode: Text.WrapAnywhere
    property int elide: Text.ElideRight
    property bool selected: listItemDelegate.highlighted || listItemDelegate.down
    property color color: selected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

    hoverEnabled: true
    width: parent ? parent.width : implicitWidth
    onClicked: {
        showPassiveNotification("Clicked on plugin: " + model.name);
    }

    contentItem: Kirigami.ActionToolBar {
        actions: [
            Kirigami.Action {
                text: model.name
                icon.name: "arrow-down-symbolic"
                enabled: false
                displayHint: Kirigami.DisplayHint.KeepVisible
            },
            Kirigami.Action {
                text: i18n("Delete this Preset")
                icon.name: "delete"
                displayHint: Kirigami.DisplayHint.AlwaysHide
                onTriggered: {
                    showPassiveNotification("Removed plugin: " + model.name);
                }
            }
        ]

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: Kirigami.Units.smallSpacing
        }

    }

}
