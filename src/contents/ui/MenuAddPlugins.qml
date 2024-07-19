import "Common.js" as Common
import EEtagsPluginName
import QtCore
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    property var streamDB

    function showMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

    parent: applicationWindow().overlay
    showCloseButton: false
    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: appWindow.height * 0.75
    onClosed: {
        status.visible = false;
    }

    ListView {
        id: listView

        Layout.fillWidth: true
        clip: true
        delegate: listDelegate
        reuseItems: true
        model: SortedPluginsNameModel

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count === 0
            text: i18n("Empty")
        }

    }

    Component {
        id: listDelegate

        Controls.ItemDelegate {
            id: listItemDelegate

            property int wrapMode: Text.WrapAnywhere
            property int elide: Text.ElideRight
            property color color: Kirigami.Theme.textColor

            hoverEnabled: false
            down: false
            width: parent ? parent.width : implicitWidth

            contentItem: RowLayout {
                Controls.Label {
                    Layout.fillWidth: true
                    text: model.translatedName
                }

                Controls.Button {
                    Layout.alignment: Qt.AlignCenter
                    icon.name: "list-add"
                    onClicked: {
                        let plugins = streamDB.plugins;
                        let index_list = [];
                        for (let n = 0; n < plugins.length; n++) {
                            if (plugins[n].startsWith(model.name)) {
                                let m = plugins[n].match(/#(\d+)$/);
                                if (m.length == 2)
                                    index_list.push(m[1]);

                            }
                        }
                        let new_id = (index_list.length === 0) ? 0 : Math.max.apply(null, index_list) + 1;
                        let new_name = model.name + "#" + new_id;
                        plugins.push(new_name);
                        streamDB.plugins = plugins;
                        showMenuStatus(i18n("Added Plugin: " + model.translatedName));
                    }
                }

            }

        }

    }

    header: ColumnLayout {
        Kirigami.SearchField {
            id: search

            Layout.fillWidth: true
            placeholderText: i18n("Search")
            onAccepted: {
                SortedPluginsNameModel.filterRegularExpression = RegExp(search.text, "i");
            }
        }

    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: status

            Layout.fillWidth: true
            visible: false
            showCloseButton: true
            Layout.maximumWidth: parent.width
        }

    }

}
