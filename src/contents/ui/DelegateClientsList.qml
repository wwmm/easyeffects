import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    required property int id
    required property int serial
    required property string name
    required property string access
    required property string api

    contentItem: Item {
        implicitWidth: delegateLayout.implicitWidth
        implicitHeight: delegateLayout.implicitHeight

        GridLayout {
            id: delegateLayout

            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.mediumSpacing
            columns: 2
            rows: 3

            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                level: 2
                text: i18n("Id")
            }

            Controls.Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                text: id
                color: Kirigami.Theme.disabledTextColor
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                level: 2
                text: i18n("Name")
            }

            Controls.Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                text: name
                color: Kirigami.Theme.disabledTextColor
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                level: 2
                text: i18n("Api")
            }

            Controls.Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                text: api
                color: Kirigami.Theme.disabledTextColor
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignRight
                level: 2
                text: i18n("Access")
            }

            Controls.Label {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                text: access
                color: Kirigami.Theme.disabledTextColor
            }
        }
    }
}
