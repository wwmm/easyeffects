import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.presets as Presets
import ee.type.presets as TypePresets
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

ColumnLayout {
    id: columnLayout

    function showPresetsMenuStatus(label) {
        status.text = label;
        status.visible = true;
    }

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
            readonly property string deviceProfileName: {
                const proxyIndex = model.index(currentIndex, 0);
                return model.data(proxyIndex, PW.ModelNodes.DeviceProfileName);
            }

            verticalPadding: 0
            text: i18n("Device")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "description"
            editable: false
            model: DB.Manager.main.visiblePage === 0 ? PW.ModelSinkDevices : PW.ModelSourceDevices
            description: i18n("Hardware Profile") + `: ${deviceProfileName}`
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
            text: i18n("Local Preset")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "name"
            editable: false
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
        }

        Controls.Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            text: i18n("Create")
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

    Kirigami.ScrollablePage {
        Layout.fillHeight: true

        Kirigami.CardsListView {
            id: streamsListView

            Layout.fillHeight: true
            clip: true
            reuseItems: true
            model: DB.Manager.main.visiblePage === 0 ? Presets.SortedAutoloadingOutputListModel : Presets.SortedAutoloadingInputListModel

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: streamsListView.count === 0
                text: i18n("Empty List")
                icon.name: "notification-empty"
            }

            delegate: Kirigami.AbstractCard {
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
                            text: i18n("Device")
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: deviceName
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Button {
                            Layout.alignment: Qt.AlignCenter | Qt.AlignRight
                            Layout.rowSpan: 4
                            icon.name: "delete"
                            onClicked: {
                                if (DB.Manager.main.visiblePage === 0)
                                    Presets.Manager.removeAutoload(1, devicePreset, deviceName, deviceProfile);
                                else if (DB.Manager.main.visiblePage === 1)
                                    Presets.Manager.removeAutoload(0, devicePreset, deviceName, deviceProfile);
                            }
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Description")
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: deviceDescription
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            horizontalAlignment: Qt.AlignRight
                            text: i18n("Hardware Profile")
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: deviceProfile
                            color: Kirigami.Theme.disabledTextColor
                        }

                        Controls.Label {
                            Layout.alignment: Qt.AlignRight
                            wrapMode: Text.WordWrap
                            text: i18n("Local Preset")
                        }

                        Controls.Label {
                            Layout.fillWidth: true
                            wrapMode: Text.WrapAnywhere
                            text: devicePreset
                            color: Kirigami.Theme.disabledTextColor
                        }
                    }
                }
            }
        }
    }

    RowLayout {
        FormCard.FormComboBoxDelegate {
            id: fallbackPreset

            Layout.fillWidth: true
            verticalPadding: 0
            text: i18n("Fallback Preset")
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

    Kirigami.InlineMessage {
        id: status

        Layout.fillWidth: true
        Layout.maximumWidth: parent.width
        visible: false
        showCloseButton: true
    }
}
