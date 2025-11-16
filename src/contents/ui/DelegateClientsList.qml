import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Controls.ItemDelegate {
    id: root

    required property int index
    required property int id
    required property int serial
    required property string name
    required property string access
    required property string api

    width: ListView.view.width
    background: Kirigami.FlexColumn {
        maximumWidth: Kirigami.Units.gridUnit * 40

        Kirigami.Separator {
            Layout.alignment: Qt.AlignBottom
            Layout.fillWidth: true
            visible: root.index !== 0
        }
    }

    contentItem: Kirigami.FlexColumn {
        maximumWidth: Kirigami.Units.gridUnit * 40

        // Port to FlexboxLayout when we can depend on Qt 6.10
        Flow {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 2
                text: root.name || i18nc("@info:placeholder", "Unknown")
            }

            Controls.Label {
                text: i18n("(id: %1)", root.id)
                opacity: 0.8
            }
        }

        Controls.Label {
            text: i18n("Access: %1", root.access)
            visible: root.access

            Layout.fillWidth: true
        }

        Controls.Label {
            text: i18n("API: %1", root.api)
            visible: root.api

            Layout.fillWidth: true
        }
    }
}
