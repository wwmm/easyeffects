import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.database as DB
import ee.presets as Presets
import ee.ui
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as Components
import org.kde.kirigamiaddons.formcard as FormCard

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

    parent: applicationWindow().overlay// qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: 0
    implicitWidth: Math.max(appWindow.width * 0.5, Kirigami.Units.gridUnit * 40)
    implicitHeight: appWindow.maxOverlayHeight // qmllint disable

    onAboutToShow: {
        pageLoader.active = true;
    }

    onAboutToHide: {
        pageLoader.active = false;
    }

    Loader {
        id: pageLoader

        height: control.height - control.header.height - control.footer.height - Kirigami.Units.largeSpacing * 5
        active: false

        source: {
            switch (DB.Manager.main.visiblePresetSheetPage) {
            case 0:
                return Qt.resolvedUrl("PresetsLocalPage.qml");
            case 1:
                return Qt.resolvedUrl("PresetsCommunityPage.qml");
            case 2:
                return Qt.resolvedUrl("PresetsAutoloadingPage.qml");
            default:
                return "";
            }
        }
    }

    header: RowLayout {
        Components.SegmentedButton {
            id: segmentedButton

            Layout.alignment: Qt.AlignHCenter

            readonly property bool hasEnoughWidth: appWindow.width >= Kirigami.Units.gridUnit * 40

            readonly property var displayHint: (!Kirigami.Settings.isMobile && hasEnoughWidth) ? Kirigami.DisplayHint.KeepVisible : Kirigami.DisplayHint.IconOnly

            actions: [
                Kirigami.Action {
                    text: i18n("Local")
                    icon.name: "system-file-manager-symbolic"
                    checkable: true
                    checked: DB.Manager.main.visiblePresetSheetPage === 0
                    displayHint: segmentedButton.displayHint
                    onTriggered: DB.Manager.main.visiblePresetSheetPage = 0
                },
                Kirigami.Action {
                    text: i18n("Community")
                    icon.name: "system-users-symbolic"
                    checkable: true
                    checked: DB.Manager.main.visiblePresetSheetPage === 1
                    displayHint: segmentedButton.displayHint
                    onTriggered: DB.Manager.main.visiblePresetSheetPage = 1
                },
                Kirigami.Action {
                    text: i18n("Autoloading")
                    icon.name: "task-recurring-symbolic"
                    checkable: true
                    checked: DB.Manager.main.visiblePresetSheetPage === 2
                    displayHint: segmentedButton.displayHint
                    onTriggered: DB.Manager.main.visiblePresetSheetPage = 2
                }
            ]
        }
    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            position: Kirigami.InlineMessage.Position.Footer
            visible: DB.Manager.main.visiblePresetSheetPage !== 2
            text: {
                if (Common.isEmpty(control.lastLoadedPresetName))
                    return i18n("No preset loaded");// qmllint disable

                const presetType = Common.isEmpty(lastLoadedCommunityPackage) ? i18n("Local") : i18n("Community"); // qmllint disable

                return `${presetType}: <strong>${control.lastLoadedPresetName}<strong>`;
            }
        }

        RowLayout {
            visible: DB.Manager.main.visiblePresetSheetPage === 2

            FormCard.FormComboBoxDelegate {
                id: fallbackPreset

                Layout.fillWidth: true
                verticalPadding: 0
                text: i18n("Fallback Preset") // qmllint disable
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: {
                    const fallbackPreset = DB.Manager.main.visiblePage === 0 ? DB.Manager.main.outputAutoloadingFallbackPreset : DB.Manager.main.inputAutoloadingFallbackPreset;
                    for (let n = 0; n < model.rowCount(); n++) {
                        const proxyIndex = model.index(n, 0);
                        const name = model.data(proxyIndex, PresetsListModel.Name);
                        if (name === fallbackPreset)
                            return n;
                    }
                    return 0;
                }
                textRole: "name"
                editable: false
                enabled: DB.Manager.main.visiblePage === 0 ? DB.Manager.main.outputAutoloadingUsesFallback : DB.Manager.main.inputAutoloadingUsesFallback
                model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
                onActivated: idx => {
                    if (DB.Manager.main.visiblePage === 0)
                        DB.Manager.main.outputAutoloadingFallbackPreset = currentText;
                    else if (DB.Manager.main.visiblePage === 1)
                        DB.Manager.main.inputAutoloadingFallbackPreset = currentText;
                }
            }

            EeSwitch {
                Layout.fillWidth: false
                Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
                isChecked: DB.Manager.main.visiblePage === 0 ? DB.Manager.main.outputAutoloadingUsesFallback : DB.Manager.main.inputAutoloadingUsesFallback
                verticalPadding: 0
                onCheckedChanged: {
                    if (DB.Manager.main.visiblePage === 0) {
                        if (isChecked !== DB.Manager.main.outputAutoloadingUsesFallback)
                            DB.Manager.main.outputAutoloadingUsesFallback = isChecked;
                    } else if (DB.Manager.main.visiblePage === 1) {
                        if (isChecked !== DB.Manager.main.inputAutoloadingUsesFallback)
                            DB.Manager.main.inputAutoloadingUsesFallback = isChecked;
                    }
                }
            }
        }
    }
}
