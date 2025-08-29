import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    required property var streamDB

    function showMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit
    showCloseButton: false
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8)
    implicitHeight: control.parent.height - 2 * (control.header.height + control.footer.height) - control.y
    onClosed: {
        status.visible = false;
    }

    ListView {
        id: listView

        clip: true
        delegate: listDelegate
        reuseItems: true
        model: TagsPluginName.SortedNameModel

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

            required property string name
            required property string translatedName
            property int wrapMode: Text.WrapAnywhere
            property int elide: Text.ElideRight
            property color color: Kirigami.Theme.textColor

            hoverEnabled: false
            down: false
            width: parent ? parent.width : implicitWidth

            contentItem: RowLayout {
                Controls.Label {
                    Layout.fillWidth: true
                    text: translatedName
                }

                Controls.Button {
                    Layout.alignment: Qt.AlignCenter
                    icon.name: "list-add"
                    onClicked: {
                        let plugins = streamDB.plugins;
                        let index_list = [];
                        for (let n = 0; n < plugins.length; n++) {
                            if (plugins[n].startsWith(name)) {
                                const m = plugins[n].match(/#(\d+)$/);
                                if (m.length === 2)
                                    index_list.push(m[1]);
                            }
                        }
                        const new_id = (index_list.length === 0) ? 0 : Math.max.apply(null, index_list) + 1;
                        const new_name = name + "#" + new_id;
                        /*
                            If the list is not empty and the user is careful protecting
                            their device with a plugin of type limiter at the last position
                            of the filter chain, we follow this behaviour trying to insert
                            the new plugin at the second to last position.

                            To do so, we first check if the new plugin is a limiter or the
                            level meter and place it directly at the last position (those
                            plugins do not need to be placed elsewhere and in most of the
                            cases the user wants them at the bottom of the pipeline).

                            If the last plugin is a limiter, we place the new plugin at
                            the second to last position.

                            If the last plugin is not a limiter, but a level meter, we still
                            try to place the new plugin before a limiter, if this limiter is in
                            the second to last position. The reason is that we still want to preserve the
                            "limiter protection" in case the last plugins are a limiter followed by a meter.
                        */
                        const limiters_and_meters = [TagsPluginName.BaseName.limiter, TagsPluginName.BaseName.maximizer, TagsPluginName.BaseName.level_meter];
                        const limiters = [TagsPluginName.BaseName.limiter, TagsPluginName.BaseName.maximizer];
                        if (plugins.length === 0) {
                            plugins.push(new_name);
                        } else if (limiters_and_meters.some(v => {
                            return v === name;
                        })) {
                            plugins.push(new_name);
                        } else if (limiters.some(v => {
                            return plugins[plugins.length - 1].startsWith(v);
                        })) {
                            plugins.splice(-1, 0, new_name);
                        } else if (plugins[plugins.length - 1].startsWith(TagsPluginName.BaseName.level_meter)) {
                            if (plugins.length >= 2) {
                                if (limiters.some(v => {
                                    return plugins[plugins.length - 2].startsWith(v);
                                }))
                                    plugins.splice(-2, 0, new_name);
                                else
                                    plugins.splice(-1, 0, new_name);
                            } else {
                                plugins.splice(-1, 0, new_name);
                            }
                        } else {
                            plugins.push(new_name);
                        }
                        streamDB.plugins = plugins;
                        showMenuStatus(i18n("Added Effect") + `: <strong>${translatedName}</strong>`);
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
                const re = Common.regExpEscape(search.text);
                TagsPluginName.SortedNameModel.filterRegularExpression = RegExp(re, "i");
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
