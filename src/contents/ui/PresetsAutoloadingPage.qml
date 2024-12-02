import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import ee.presets as Presets
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

            verticalPadding: 0
            text: i18n("Device")
            displayMode: FormCard.FormComboBoxDelegate.ComboBox
            currentIndex: 0
            textRole: "description"
            editable: false
            model: DB.Manager.main.visibleAutoloadingTab === 0 ? PW.ModelSinkDevices : PW.ModelSourceDevices
            onActivated: (idx) => {
            }
        }

        Kirigami.Icon {
            source: DB.Manager.main.visibleAutoloadingTab === 1 ? "audio-input-microphone-symbolic" : "audio-speakers-symbolic"
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
            model: DB.Manager.main.visibleAutoloadingTab === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
            onActivated: (idx) => {
            }
        }

        Controls.Button {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            text: i18n("Create")
            icon.name: "list-add-symbolic"
            onClicked: {
                showPassiveNotification("creating profile");
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
            model: []

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                visible: streamsListView.count === 0
                text: i18n("Empty List")
                icon.name: "notification-empty"
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
                const fallbackPreset = DB.Manager.main.visibleAutoloadingTab === 0 ? DB.Manager.main.outputAutoloadingFallbackPreset : DB.Manager.main.inputAutoloadingFallbackPreset;
                for (let n = 0; n < model.rowCount(); n++) {
                    const proxyIndex = model.index(n, 0);
                    const name = model.data(proxyIndex, 256); // 256 = Qt::UserRole
                    if (name === fallbackPreset)
                        return n;

                }
                return 0;
            }
            textRole: "name"
            editable: false
            enabled: DB.Manager.main.visibleAutoloadingTab === 0 ? DB.Manager.main.outputAutoloadingUsesFallback : DB.Manager.main.inputAutoloadingUsesFallback
            model: DB.Manager.main.visibleAutoloadingTab === 0 ? Presets.SortedOutputListModel : Presets.SortedInputListModel
            onActivated: (idx) => {
                if (DB.Manager.main.visibleAutoloadingTab === 0)
                    DB.Manager.main.outputAutoloadingFallbackPreset = currentText;
                else if (DB.Manager.main.visibleAutoloadingTab === 1)
                    DB.Manager.main.inputAutoloadingFallbackPreset = currentText;
            }
        }

        EeSwitch {
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            isChecked: DB.Manager.main.visibleAutoloadingTab === 0 ? DB.Manager.main.outputAutoloadingUsesFallback : DB.Manager.main.inputAutoloadingUsesFallback
            verticalPadding: 0
            onCheckedChanged: {
                if (DB.Manager.main.visibleAutoloadingTab === 0) {
                    if (isChecked !== DB.Manager.main.outputAutoloadingUsesFallback)
                        DB.Manager.main.outputAutoloadingUsesFallback = isChecked;

                } else if (DB.Manager.main.visibleAutoloadingTab === 1) {
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
