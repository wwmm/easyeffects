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
    required property string version
    required property string description
    required property string filename

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

        Flow {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Heading {
                level: 2
                text: root.name || i18nc("@info:placeholder", "Unknown")
            }

            Controls.Label {
                text: i18n("(id: %1, version: %2)", root.id, root.version || i18nc("@info:placeholder", "Not set"))
                opacity: 0.8
            }
        }

        Controls.Label {
            text: root.description || i18nc("@info:placeholder", "No description set")
        }
    }
}
