import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.presets as Presets
import ee.type.presets as TypePresets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: root

    function showPresetsMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

    header: GridLayout {
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
            readonly property string deviceProfileName: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.DeviceProfileName);
            }

            verticalPadding: 0
            text: i18n("Device") // qmllint disable
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "description"
            editable: false
            model: DB.Manager.main.visiblePage === 0 ? PW.ModelSinkDevices : PW.ModelSourceDevices
            description: i18n("Hardware Profile") + `: ${deviceProfileName}` // qmllint disable
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
            text: i18n("Local Preset") // qmllint disable
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
            onClicked: {
                const deviceId = device.deviceId;
                const deviceName = device.deviceName;
                const deviceDescription = device.currentText;
                const presetName = preset.currentText;
                const deviceRoute = device.deviceProfileName;
                if (DB.Manager.main.visiblePage === 0)
                    Presets.Manager.addAutoload(1, presetName, deviceName, deviceDescription, deviceRoute);
                else if (DB.Manager.main.visiblePage === 1)
                    Presets.Manager.addAutoload(0, presetName, deviceName, deviceDescription, deviceRoute);
            }
        }
    }

    ColumnLayout {
        id: columnLayout

        anchors.fill: parent

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
                                text: i18n("Hardware Profile") // qmllint disable
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
                                text: i18n("Local Preset") // qmllint disable
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

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: status

            Layout.fillWidth: true
            Layout.maximumWidth: parent.width
            visible: false
            showCloseButton: true
        }

        RowLayout {
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
                        const name = model.data(proxyIndex, TypePresets.ListModel.Name);
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
