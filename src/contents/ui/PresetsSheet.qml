import "Common.js" as Common
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    readonly property string lastLoadedPresetName: {
        if (DB.Manager.main.visiblePage === 0)
            return DB.Manager.main.lastLoadedOutputPreset;
        else if (DB.Manager.main.visiblePage === 1)
            return DB.Manager.main.lastLoadedInputPreset;
        return "";
    }
    readonly property string lastLoadedCommunityPackage: {
        if (DB.Manager.main.visiblePage === 0)
            return DB.Manager.main.lastLoadedOutputCommunityPackage;
        else if (DB.Manager.main.visiblePage === 1)
            return DB.Manager.main.lastLoadedInputCommunityPackage;
        return "";
    }

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit
    implicitWidth: Math.min(stackView.implicitWidth, appWindow.width * 0.8)
    implicitHeight: control.parent.height - (control.header.height + control.footer.height) - control.y
    onVisibleChanged: {
        if (control.visible) {
            switch (DB.Manager.main.visiblePresetSheetPage) {
            case 0:
                stackView.replace("qrc:ui/PresetsLocalPage.qml");
                break;
            case 1:
                stackView.replace("qrc:ui/PresetsCommunityPage.qml");
                break;
            case 2:
                stackView.replace("qrc:ui/PresetsAutoloadingPage.qml");
                break;
            default:
                null;
            }
        }
    }

    Controls.StackView {
        id: stackView

        implicitWidth: Math.max(appWindow.width * 0.5, Kirigami.Units.gridUnit * 40)
        implicitHeight: control.height - (control.header.height + control.footer.height) - control.y
    }

    header: Kirigami.ActionToolBar {
        id: tabbar

        alignment: Qt.AlignHCenter
        position: Controls.ToolBar.Header
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
                    stackView.replace("qrc:ui/PresetsCommunityPage.qml");
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
                    stackView.replace("qrc:ui/PresetsAutoloadingPage.qml");
                    DB.Manager.main.visiblePresetSheetPage = 2;
                }
            }
        ]
    }

    footer: Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        position: Kirigami.InlineMessage.Position.Footer
        visible: DB.Manager.main.visiblePresetSheetPage !== 2
        text: {
            if (Common.isEmpty(lastLoadedPresetName))
                return i18n("No Preset Loaded");

            const presetType = Common.isEmpty(lastLoadedCommunityPackage) ? i18n("Local") : i18n("Community");

            return `${presetType}: <strong>${lastLoadedPresetName}<strong>`;
        }
    }
}
