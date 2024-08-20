import EEpw
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: pwPage

    padding: 0
    Component.onCompleted: {
        panelModel.append({
            "title": i18n("General"),
            "icon": "documentinfo-symbolic"
        });
        panelModel.append({
            "title": i18n("Preset Autoloading"),
            "icon": "bookmarks-symbolic"
        });
        panelModel.append({
            "title": i18n("Modules"),
            "icon": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Clients"),
            "icon": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Test Signal"),
            "icon": "waveform-symbolic"
        });
    }

    Component {
        id: generalPage

        FormCard.FormCardPage {
            function comboFindRow(model, nodeName) {
                let row = -1;
                let nodeIndex = ModelNodes.getModelIndexByName(nodeName);
                let modelRow = model.mapFromSource(nodeIndex).row;
                if (modelRow >= 0)
                    row = modelRow;

                return row;
            }

            function updateInputDevComboSelection() {
                let deviceName = useDefaultInputDevice.isChecked ? EEpwManager.defaultInputDeviceName : EEdbStreamInputs.inputDevice;
                let comboRow = comboFindRow(ModelSourceDevices, deviceName);
                if (comboRow !== -1)
                    comboInputDevice.currentIndex = comboRow;

            }

            function updateOutputDevComboSelection() {
                let deviceName = useDefaultOutputDevice.isChecked ? EEpwManager.defaultOutputDeviceName : EEdbStreamOutputs.outputDevice;
                let comboRow = comboFindRow(ModelSinkDevices, deviceName);
                if (comboRow !== -1)
                    comboOutputDevice.currentIndex = comboRow;

            }

            Connections {
                function onDataChanged() {
                    updateInputDevComboSelection();
                }

                target: ModelSourceDevices
            }

            Connections {
                function onDataChanged() {
                    updateOutputDevComboSelection();
                }

                target: ModelSinkDevices
            }

            FormCard.FormHeader {
                title: i18n("Device Management")
            }

            FormCard.FormSectionText {
                text: i18n("It's recommended to NOT set Easy Effects Sink/Source as Default Device in external applications (e.g. Gnome Settings and Plasma System Settings)")
            }

            FormCard.FormCard {
                EeSwitch {
                    id: useDefaultInputDevice

                    label: i18n("Use Default Input")
                    isChecked: EEdbStreamInputs.useDefaultInputDevice
                    onCheckedChanged: {
                        if (isChecked)
                            updateInputDevComboSelection();

                        if (isChecked !== EEdbStreamInputs.useDefaultInputDevice)
                            EEdbStreamInputs.useDefaultInputDevice = isChecked;

                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboInputDevice

                    text: i18n("Name")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: ModelSourceDevices
                    textRole: "description"
                    enabled: !EEdbStreamInputs.useDefaultInputDevice
                    onActivated: (idx) => {
                        let proxyIndex = ModelSourceDevices.index(idx, 0);
                        let sourceIndex = ModelSourceDevices.mapToSource(proxyIndex);
                        let nodeName = ModelNodes.getNodeName(sourceIndex.row);
                        if (EEdbStreamInputs.inputDevice !== nodeName)
                            EEdbStreamInputs.inputDevice = nodeName;

                    }
                }

            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: useDefaultOutputDevice

                    label: i18n("Use Default Output")
                    isChecked: EEdbStreamOutputs.useDefaultOutputDevice
                    onCheckedChanged: {
                        if (isChecked)
                            updateOutputDevComboSelection();

                        if (isChecked !== EEdbStreamOutputs.useDefaultOutputDevice)
                            EEdbStreamOutputs.useDefaultOutputDevice = isChecked;

                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboOutputDevice

                    text: i18n("Name")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: ModelSinkDevices
                    textRole: "description"
                    enabled: !EEdbStreamOutputs.useDefaultOutputDevice
                    onActivated: (idx) => {
                        let proxyIndex = ModelSinkDevices.index(idx, 0);
                        let sourceIndex = ModelSinkDevices.mapToSource(proxyIndex);
                        let nodeName = ModelNodes.getNodeName(sourceIndex.row);
                        if (EEdbStreamOutputs.outputDevice !== nodeName)
                            EEdbStreamOutputs.outputDevice = nodeName;

                    }
                }

            }

            FormCard.FormHeader {
                title: i18n("Server Information")
            }

            FormCard.FormCard {
                FormCard.FormTextDelegate {
                    text: i18n("Header Version")
                    description: EEpwManager.headerVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Library Version")
                    description: EEpwManager.libraryVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Runtime Version")
                    description: EEpwManager.runtimeVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Sampling Rate")
                    description: EEpwManager.defaultClockRate + " Hz"
                }

                FormCard.FormTextDelegate {
                    text: i18n("Minimum Quantum")
                    description: EEpwManager.defaultMinQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Maximum Quantum")
                    description: EEpwManager.defaultMaxQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Quantum")
                    description: EEpwManager.defaultQuantum
                }

            }

        }

    }

    Component {
        id: modulesPage

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: modulesListView

                clip: true
                reuseItems: true
                model: ModelModules

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: modulesListView.count === 0
                    text: i18n("No Modules")
                }

                delegate: DelegateModulesList {
                }

            }

        }

    }

    Component {
        id: clientsPage

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: clientsListView

                clip: true
                reuseItems: true
                model: ModelClients

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: clientsListView.count === 0
                    text: i18n("No Clients")
                }

                delegate: DelegateClientsList {
                }

            }

        }

    }

    GridLayout {
        columns: 3
        rows: 1
        columnSpacing: 0
        anchors.fill: parent

        ListView {
            id: panelListView

            Layout.fillHeight: true
            Layout.minimumWidth: Math.round(pwPage.width * 0.15)
            clip: true
            reuseItems: true

            model: ListModel {
                id: panelModel
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                property int elide: Text.ElideRight

                width: parent ? parent.width : implicitWidth
                hoverEnabled: true
                highlighted: ListView.isCurrentItem
                onClicked: {
                    ListView.view.currentIndex = index;
                    switch (index) {
                    case 0:
                        panelStack.replace(generalPage);
                        break;
                    case 1:
                        panelStack.replace(autoloadingPage);
                        break;
                    case 2:
                        panelStack.replace(modulesPage);
                        break;
                    case 3:
                        panelStack.replace(clientsPage);
                        break;
                    case 4:
                        panelStack.replace(testSignalPage);
                        break;
                    default:
                        console.log("pipewire page stackview: invalid index");
                    }
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: model.icon
                        Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.alignment: Qt.AlignLeft
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: model.title
                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WordWrap
                    }

                }

            }

        }

        Kirigami.Separator {
            Layout.fillHeight: true
            visible: true
        }

        Controls.StackView {
            id: panelStack

            Layout.fillHeight: true
            Layout.fillWidth: true
            initialItem: generalPage
        }

    }

}
