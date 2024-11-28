import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true

    ColumnLayout {
        Layout.maximumWidth: appWindow.width * 0.5 // Kirigami.Units.gridUnit * 40
        Layout.preferredWidth: Layout.maximumWidth
        Layout.preferredHeight: Layout.maximumHeight

        Controls.StackView {
            id: stackView

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredHeight: control.height - control.header.height

            initialItem: PresetsLocalPage {
            }

        }

    }

    header: Kirigami.ActionToolBar {
        id: tabbar

        alignment: Qt.AlignHCenter
        actions: [
            Kirigami.Action {
                displayHint: Kirigami.DisplayHint.KeepVisible
                text: i18n("Local")
                icon.name: "system-file-manager-symbolic"
                checkable: true
                checked: DB.Manager.main.visiblePresetSheetPage === 0
                onTriggered: {
                    stackView.replace("qrc:ui/PresetsLocalPage.qml");
                    DB.Manager.main.visiblePresetSheetPage = 0;
                }
            },
            Kirigami.Action {
                displayHint: Kirigami.DisplayHint.KeepVisible
                text: i18n("Community")
                icon.name: "system-users-symbolic"
                checkable: true
                checked: DB.Manager.main.visiblePresetSheetPage === 1
                onTriggered: {
                    // stackView.replace("qrc:ui/PageStreamsEffects.qml");
                    DB.Manager.main.visiblePresetSheetPage = 1;
                }
            },
            Kirigami.Action {
                displayHint: Kirigami.DisplayHint.KeepVisible
                text: i18n("Autoloading")
                icon.name: "task-recurring-symbolic"
                checkable: true
                checked: DB.Manager.main.visiblePresetSheetPage === 2
                onTriggered: {
                    // stackView.replace("qrc:ui/PipeWirePage.qml");
                    DB.Manager.main.visiblePresetSheetPage = 2;
                }
            }
        ]
    }

}
