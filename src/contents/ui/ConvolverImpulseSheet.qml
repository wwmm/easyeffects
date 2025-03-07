import "Common.js" as Common
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.presets as Presets
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var pluginDB

    function showImpulseMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit
    implicitWidth: Math.max(Kirigami.Units.gridUnit * 40, appWindow.width * 0.5)
    implicitHeight: control.parent.height - 2 * (control.header.height + control.footer.height) - control.y
    onClosed: {
        status.visible = false;
    }

    FileDialog {
        id: fileDialog

        fileMode: FileDialog.OpenFiles
        currentFolder: StandardPaths.standardLocations(StandardPaths.DownloadLocation)[0]
        nameFilters: ["IRS (*.irs)", "WAVE (*.wav)"]
        onAccepted: {
            if (Presets.Manager.importImpulses(fileDialog.selectedFiles) === 0)
                showImpulseMenuStatus(i18n("Preset files imported!"));
            else
                showImpulseMenuStatus(i18n("Failed to import the impulse file!"));
        }
    }

    ColumnLayout {
        height: control.height - (control.header.height + control.footer.height) - control.y

        Kirigami.SearchField {
            id: search

            Layout.fillWidth: true
            placeholderText: i18n("Search")
            onAccepted: {
                const re = Common.regExpEscape(search.text);
                Presets.SortedImpulseListModel.filterRegularExpression = RegExp(re, "i");
            }
        }

        ListView {
            id: listView

            clip: true
            reuseItems: true
            model: Presets.SortedImpulseListModel
            Layout.fillWidth: true
            Layout.fillHeight: true

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: listView.count === 0
                text: i18n("Empty")
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                required property string name
                required property string path

                hoverEnabled: true
                width: listView.width
                onClicked: {
                    pluginDB.kernelName = name;
                    showImpulseMenuStatus(i18n("Loaded Impulse: %1", name));
                }

                contentItem: RowLayout {
                    Controls.Label {
                        text: name
                    }

                    Kirigami.ActionToolBar {
                        alignment: Qt.AlignRight
                        actions: [
                            Kirigami.Action {
                                text: i18n("Delete this Impulse")
                                icon.name: "delete"
                                displayHint: Kirigami.DisplayHint.AlwaysHide
                                onTriggered: {
                                    if (Presets.Manager.removeImpulseFile(path) === true)
                                        showImpulseMenuStatus(i18n("Removed Impulse: %1", name));
                                    else
                                        showImpulseMenuStatus(i18n("Failed to Remove: %1", name));
                                }
                            }
                        ]
                    }

                }

            }

        }

    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: status

            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            visible: false
            showCloseButton: true
        }

    }

    header: Kirigami.ActionToolBar {
        alignment: Qt.AlignHCenter
        position: Controls.ToolBar.Header
        actions: [
            Kirigami.Action {
                displayHint: Kirigami.DisplayHint.KeepVisible
                text: i18n("Import Impulse")
                icon.name: "document-import-symbolic"
                onTriggered: {
                    fileDialog.open();
                }
            }
        ]
    }

}
