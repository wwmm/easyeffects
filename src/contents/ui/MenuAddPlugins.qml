/**
 * Copyright © 2025-2026 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import "Common.js" as Common
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

Kirigami.Dialog {
    id: control

    required property var streamDB

    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnReleaseOutside
    focus: true
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8) // qmllint disable
    implicitHeight: Math.min(Kirigami.Units.gridUnit * 40, Math.round(Controls.ApplicationWindow.window.height * 0.8))
    modal: true
    bottomPadding: 1
    anchors.centerIn: parent

    ListView {
        id: listView

        currentIndex: -1
        clip: true
        delegate: listDelegate
        reuseItems: true
        model: TagsPluginName.SortedNameModel

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - (Kirigami.Units.largeSpacing * 4)
            visible: listView.count === 0
            text: i18n("Empty List") // qmllint disable
        }
    }

    Component {
        id: listDelegate

        Delegates.RoundedItemDelegate {
            id: listItemDelegate

            required property string name
            required property string translatedName
            property int wrapMode: Text.WrapAnywhere
            property int elide: Text.ElideRight
            property color color: Kirigami.Theme.textColor

            hoverEnabled: true
            highlighted: false
            down: false
            width: parent ? parent.width : implicitWidth

            function addFilter(): void {
                button.clicked();
            }

            contentItem: RowLayout {
                Controls.Label {
                    Layout.fillWidth: true
                    text: listItemDelegate.translatedName
                }

                Controls.Button {
                    id: button

                    icon.name: "list-add"
                    down: listItemDelegate.ListView.isCurrentItem

                    Layout.alignment: Qt.AlignCenter

                    Controls.ToolTip.text: i18n("Add %1", listItemDelegate.translatedName)
                    Controls.ToolTip.visible: hovered

                    onClicked: {
                        let plugins = control.streamDB.plugins;
                        let index_list = [];

                        for (let n = 0; n < plugins.length; n++) {
                            if (plugins[n].startsWith(listItemDelegate.name)) {
                                const m = plugins[n].match(/#(\d+)$/);
                                if (m.length === 2)
                                    index_list.push(m[1]);
                            }
                        }

                        const new_id = (index_list.length === 0) ? 0 : Math.max.apply(null, index_list) + 1;
                        const new_name = listItemDelegate.name + "#" + new_id;

                        /**
                         * If the list is not empty and the user is careful
                         * protecting their device with a plugin of type
                         * limiter at the last position of the filter chain, we
                         * follow this behaviour trying to insert the new
                         * plugin at the second to last position.
                         *
                         * To do so, we first check if the new plugin is a
                         * limiter or the level meter and place it directly at
                         * the last position (those plugins do not need to be
                         * placed elsewhere and in most of the cases the user
                         * wants them at the bottom of the pipeline).
                         *
                         * If the last plugin is a limiter, we place the new
                         * plugin at the second to last position.
                         *
                         * If the last plugin is not a limiter, but a level
                         * meter, we still try to place the new plugin before a
                         * limiter, if this limiter is in the second to last
                         * position. The reason is that we still want to
                         * preserve the "limiter protection" in case the last
                         * plugins are a limiter followed by a meter.
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

                        control.streamDB.plugins = plugins;

                        appWindow.showStatus(i18n("Added a new effect to the pipeline: %1", `<strong>${listItemDelegate.translatedName}</strong>`), Kirigami.MessageType.Positive); // qmllint disable
                    }
                }
            }
        }
    }

    header: ColumnLayout {
        spacing: 0

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Kirigami.SearchField {
                id: search

                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.smallSpacing
                Layout.rightMargin: 0

                focus: true
                placeholderText: i18n("Search") // qmllint disable
                autoAccept: false
                onTextChanged: {
                    const re = Common.regExpEscape(search.text);
                    TagsPluginName.SortedNameModel.filterRegularExpression = RegExp(re, "i");
                }

                onAccepted: {
                    listView.currentItem?.addFilter(); // qmllint disable
                }

                Keys.onDownPressed: listView.incrementCurrentIndex()
                Keys.onUpPressed: listView.decrementCurrentIndex()
            }

            Controls.ToolButton {
                text: i18nc("@action:button", "Close")
                icon.name: 'dialog-close-symbolic'
                display: Controls.ToolButton.IconOnly
                onClicked: control.close()
                Layout.margins: Kirigami.Units.smallSpacing
                Layout.leftMargin: 0
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }
    }
}
