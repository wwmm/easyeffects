import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit
    implicitWidth: Math.min(gridLayout.implicitWidth, appWindow.width * 0.8) + 4 * Kirigami.Units.iconSizes.large
    implicitHeight: Math.min(2 * control.header.height + gridLayout.implicitHeight, control.parent.height - 2 * control.header.height - control.y)

    GridLayout {
        id: gridLayout

        columns: 2
        columnSpacing: Kirigami.Units.gridUnit

        Kirigami.Chip {
            text: "F1"
            closable: false
            checkable: false
            down: false
            hoverEnabled: false
            Layout.alignment: Qt.AlignRight
        }

        Controls.Label {
            text: i18n("Show Help")
        }

        Kirigami.Chip {
            text: "F11"
            closable: false
            checkable: false
            down: false
            hoverEnabled: false
            Layout.alignment: Qt.AlignRight
        }

        Controls.Label {
            text: i18n("Fullscreen / Restore from fullscreen")
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            Kirigami.Chip {
                text: "Ctrl"
                checkable: false
                down: false
                hoverEnabled: false
                closable: false
            }

            Controls.Label {
                text: i18n("+")
            }

            Kirigami.Chip {
                text: "W"
                closable: false
                checkable: false
                down: false
                hoverEnabled: false
            }

        }

        Controls.Label {
            text: i18n("Close the Window")
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            Kirigami.Chip {
                text: "Ctrl"
                closable: false
                checkable: false
                down: false
                hoverEnabled: false
            }

            Controls.Label {
                text: i18n("+")
            }

            Kirigami.Chip {
                text: "Q"
                closable: false
                checkable: false
                down: false
                hoverEnabled: false
            }

        }

        Controls.Label {
            text: i18n("Quit EasyEffects")
        }

    }

    header: RowLayout {
        Kirigami.Icon {
            id: headerIcon

            source: "configure-shortcuts-symbolic"
        }

        Kirigami.Heading {
            Layout.fillWidth: true
            text: i18n("Shortcuts")
        }

    }

}
