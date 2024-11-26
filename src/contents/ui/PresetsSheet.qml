import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay
    implicitWidth: Kirigami.Units.gridUnit * 50
    implicitHeight: appWindow.height * 0.75

    StackLayout {
        id: stackLayout

        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        //todo: replace the test rectangles by the proper section codes
        Rectangle {
            color: 'teal'
            implicitWidth: parent.width
            implicitHeight: 200
        }

        Rectangle {
            color: 'plum'
            implicitWidth: parent.width
            implicitHeight: 200
        }

        Rectangle {
            color: 'red'
            implicitWidth: parent.width
            implicitHeight: 200
        }

    }

    header: RowLayout {
        anchors.fill: parent

        Controls.TabBar {
            id: tabBar

            position: Controls.TabBar.Header
            Layout.alignment: Qt.AlignHCenter

            Controls.TabButton {
                text: i18n("Local")
                icon.name: "system-file-manager-symbolic"
            }

            Controls.TabButton {
                text: i18n("Community")
                icon.name: "system-users-symbolic"
            }

            Controls.TabButton {
                text: i18n("Autoloading")
                icon.name: "task-recurring-symbolic"
            }

        }

    }

}
