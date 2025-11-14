import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.presets as Presets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: columnLayout

    GridLayout {
        columns: 2

        FormCard.FormComboBoxDelegate {
            id: device

            readonly property int deviceId: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.DeviceId);
            }
            readonly property string deviceName: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.Name);
            }

            property string deviceRouteDescription

            verticalPadding: 0
            text: i18n("Device") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "description"
            editable: false
            model: DB.Manager.main.visiblePage === 0 ? PW.ModelSinkDevices : PW.ModelSourceDevices
            description: `${i18n("Route")}: ${deviceRouteDescription}` // qmllint disable

            function updateRouteDescription() {
                const proxyIndex = model.index(currentIndex, 0);

                return model.data(proxyIndex, PW.ModelNodes.DeviceRouteDescription);
            }

            onCurrentIndexChanged: {
                device.deviceRouteDescription = device.updateRouteDescription();
            }

            Connections {
                target: device.model

                function onModelReset() {
                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onRowsInserted() {
                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onRowsRemoved() {
                    device.deviceRouteDescription = device.updateRouteDescription();
                }
                function onDataChanged() {
                    device.deviceRouteDescription = device.updateRouteDescription();
                }
            }
        }

        Kirigami.Icon {
            source: DB.Manager.main.visiblePage === 1 ? "audio-input-microphone-symbolic" : "audio-speakers-symbolic"
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
            Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
        }

        FormCard.FormComboBoxDelegate {
            id: preset

            verticalPadding: 0
            text: i18n("Local preset") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "name"
            editable: false
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
        }

        Controls.Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            text: i18n("Create") // qmllint disable
            icon.name: "list-add-symbolic"
            Controls.ToolTip.text: i18n("Create a new autoloading preset")
            Controls.ToolTip.visible: hovered
            onClicked: {
                const deviceId = device.deviceId;
                const deviceName = device.deviceName;
                const deviceDescription = device.currentText;
                const presetName = preset.currentText;
                const deviceRoute = device.deviceRouteDescription;
                if (DB.Manager.main.visiblePage === 0)
                    Presets.Manager.addAutoload(1, presetName, deviceName, deviceDescription, deviceRoute);
                else if (DB.Manager.main.visiblePage === 1)
                    Presets.Manager.addAutoload(0, presetName, deviceName, deviceDescription, deviceRoute);
            }
        }
    }

    RowLayout {
        id: listviewRow

        Kirigami.CardsListView {
            id: streamsListView

            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            reuseItems: true
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedAutoloadingOutputListModel : Presets.SortedAutoloadingInputListModel
            Controls.ScrollBar.vertical: listViewScrollBar

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: streamsListView.count === 0
                text: i18n("Empty List") // qmllint disable
                icon.name: "notification-empty"
            }

            delegate: Kirigami.AbstractCard {
                id: abstractCard

                required property string deviceName
                required property string deviceDescription
                required property string deviceProfile
                required property string devicePreset

                contentItem: Item {
                    implicitWidth: delegateLayout.implicitWidth
                    implicitHeight: delegateLayout.implicitHeight

                    GridLayout {
                        id: delegateLayout

                        columns: 3
                        rows: 4

                        anchors {
                            left: parent.left
                            top: parent.top
                            right: parent.right
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Device") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: abstractCard.deviceName
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Button {
                            Layout.alignment: Qt.AlignCenter | Qt.AlignRight
                            Layout.rowSpan: 4
                            icon.name: "delete"
                            Controls.ToolTip.text: i18n("Remove this autoloading preset")
                            Controls.ToolTip.visible: hovered
                            onClicked: {
                                if (DB.Manager.main.visiblePage === 0)
                                    Presets.Manager.removeAutoload(1, abstractCard.devicePreset, abstractCard.deviceName, abstractCard.deviceProfile);
                                else if (DB.Manager.main.visiblePage === 1)
                                    Presets.Manager.removeAutoload(0, abstractCard.devicePreset, abstractCard.deviceName, abstractCard.deviceProfile);
                            }
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Description") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: abstractCard.deviceDescription
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Route") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: abstractCard.deviceProfile
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            text: i18n("Local preset") // qmllint disable
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: abstractCard.devicePreset
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
            }
        }

        Controls.ScrollBar {
            id: listViewScrollBar

            parent: listviewRow
            Layout.fillHeight: true
        }
    }
}
