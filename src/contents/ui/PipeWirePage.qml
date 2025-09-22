import "Common.js" as Common
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
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
        panelListView.currentIndex = DB.Manager.main.visiblePipeWirePage;
        switch (DB.Manager.main.visiblePipeWirePage) {
        case 0:
            panelStack.replace(generalPage);
            break;
        case 1:
            panelStack.replace(modulesPage);
            break;
        case 2:
            panelStack.replace(clientsPage);
            break;
        case 3:
            panelStack.replace(testSignalPage);
            break;
        default:
            null;
        }
    }

    Component {
        id: generalPage

        FormCard.FormCardPage {
            function comboFindRow(model, nodeName) {
                let row = -1;
                const nodeIndex = PW.ModelNodes.getModelIndexByName(nodeName);
                const modelRow = model.mapFromSource(nodeIndex).row;
                if (modelRow >= 0)
                    row = modelRow;

                return row;
            }

            function updateInputDevComboSelection() {
                const deviceName = useDefaultInputDevice.isChecked ? PW.Manager.defaultInputDeviceName : DB.Manager.streamInputs.inputDevice;
                const comboRow = comboFindRow(PW.ModelSourceDevices, deviceName);
                if (comboRow !== -1)
                    comboInputDevice.currentIndex = comboRow;
            }

            function updateOutputDevComboSelection() {
                const deviceName = useDefaultOutputDevice.isChecked ? PW.Manager.defaultOutputDeviceName : DB.Manager.streamOutputs.outputDevice;
                const comboRow = comboFindRow(PW.ModelSinkDevices, deviceName);
                if (comboRow !== -1)
                    comboOutputDevice.currentIndex = comboRow;
            }

            Connections {
                function onDataChanged() {
                    updateInputDevComboSelection();
                }

                target: PW.ModelSourceDevices
            }

            Connections {
                function onDataChanged() {
                    updateOutputDevComboSelection();
                }

                target: PW.ModelSinkDevices
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
                    isChecked: DB.Manager.streamInputs.useDefaultInputDevice
                    onCheckedChanged: {
                        if (isChecked)
                            updateInputDevComboSelection();

                        if (isChecked !== DB.Manager.streamInputs.useDefaultInputDevice)
                            DB.Manager.streamInputs.useDefaultInputDevice = isChecked;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboInputDevice

                    text: i18n("Name")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: PW.ModelSourceDevices
                    textRole: "description"
                    enabled: !DB.Manager.streamInputs.useDefaultInputDevice
                    onActivated: idx => {
                        const proxyIndex = PW.ModelSourceDevices.index(idx, 0);
                        const sourceIndex = PW.ModelSourceDevices.mapToSource(proxyIndex);
                        const nodeName = PW.ModelNodes.getNodeName(sourceIndex.row);
                        if (DB.Manager.streamInputs.inputDevice !== nodeName) {
                            if (!Common.isEmpty(nodeName))
                                DB.Manager.streamInputs.inputDevice = nodeName;
                        }
                    }
                }
            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: useDefaultOutputDevice

                    label: i18n("Use Default Output")
                    isChecked: DB.Manager.streamOutputs.useDefaultOutputDevice
                    onCheckedChanged: {
                        if (isChecked)
                            updateOutputDevComboSelection();

                        if (isChecked !== DB.Manager.streamOutputs.useDefaultOutputDevice)
                            DB.Manager.streamOutputs.useDefaultOutputDevice = isChecked;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboOutputDevice

                    text: i18n("Name")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: PW.ModelSinkDevices
                    textRole: "description"
                    enabled: !DB.Manager.streamOutputs.useDefaultOutputDevice
                    onActivated: idx => {
                        const proxyIndex = PW.ModelSinkDevices.index(idx, 0);
                        const sourceIndex = PW.ModelSinkDevices.mapToSource(proxyIndex);
                        const nodeName = PW.ModelNodes.getNodeName(sourceIndex.row);
                        if (DB.Manager.streamOutputs.outputDevice !== nodeName) {
                            if (!Common.isEmpty(nodeName))
                                DB.Manager.streamOutputs.outputDevice = nodeName;
                        }
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Server Information")
            }

            FormCard.FormCard {
                FormCard.FormTextDelegate {
                    text: i18n("Header Version")
                    description: PW.Manager.headerVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Library Version")
                    description: PW.Manager.libraryVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Runtime Version")
                    description: PW.Manager.runtimeVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Sampling Rate")
                    description: PW.Manager.defaultClockRate + " Hz"
                }

                FormCard.FormTextDelegate {
                    text: i18n("Minimum Quantum")
                    description: PW.Manager.defaultMinQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Maximum Quantum")
                    description: PW.Manager.defaultMaxQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Quantum")
                    description: PW.Manager.defaultQuantum
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
                model: PW.ModelModules

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: modulesListView.count === 0
                    text: i18n("No Modules")
                }

                delegate: DelegateModulesList {}
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
                model: PW.ModelClients

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: clientsListView.count === 0
                    text: i18n("No Clients")
                }

                delegate: DelegateClientsList {}
            }
        }
    }

    Component {
        id: testSignalPage

        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("State")
            }

            FormCard.FormCard {
                EeSwitch {
                    id: enableTestSignals

                    label: i18n("Enabled")
                    isChecked: DB.Manager.testSignals.enable
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.testSignals.enable)
                            DB.Manager.testSignals.enable = isChecked;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Channels")
            }

            FormCard.FormCard {
                enabled: enableTestSignals.isChecked

                FormCard.FormRadioDelegate {
                    id: leftChannel

                    text: i18n("Left")
                    checked: DB.Manager.testSignals.channels === 0
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.channels)
                            DB.Manager.testSignals.channels = 0;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: rightChannel

                    text: i18n("Right")
                    checked: DB.Manager.testSignals.channels === 1
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.channels)
                            DB.Manager.testSignals.channels = 1;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: bothChannels

                    text: i18n("Both")
                    checked: DB.Manager.testSignals.channels === 2
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.channels)
                            DB.Manager.testSignals.channels = 2;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Waveform")
            }

            FormCard.FormCard {
                enabled: enableTestSignals.isChecked

                FormCard.FormRadioDelegate {
                    id: sineWave

                    text: i18n("Sine Wave")
                    checked: DB.Manager.testSignals.signalType === 0
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.signalType)
                            DB.Manager.testSignals.signalType = 0;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: whiteNoise

                    text: i18n("White Noise")
                    checked: DB.Manager.testSignals.signalType === 1
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.signalType)
                            DB.Manager.testSignals.signalType = 1;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: pinkNoise

                    text: i18n("Pink Noise")
                    checked: DB.Manager.testSignals.signalType === 2
                    onCheckedChanged: {
                        if (checked !== DB.Manager.testSignals.signalType)
                            DB.Manager.testSignals.signalType = 2;
                    }
                }

                EeSpinBox {
                    id: frequency

                    label: i18n("Frequency")
                    from: 10
                    to: 22000
                    value: DB.Manager.testSignals.frequency
                    decimals: 0
                    stepSize: 1
                    unit: "Hz"
                    enabled: sineWave.checked
                    onValueModified: v => {
                        if (v !== DB.Manager.testSignals.frequency)
                            DB.Manager.testSignals.frequency = v;
                    }
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
                        DB.Manager.main.visiblePipeWirePage = 0;
                        break;
                    case 1:
                        panelStack.replace(modulesPage);
                        DB.Manager.main.visiblePipeWirePage = 1;
                        break;
                    case 2:
                        panelStack.replace(clientsPage);
                        DB.Manager.main.visiblePipeWirePage = 2;
                        break;
                    case 3:
                        panelStack.replace(testSignalPage);
                        DB.Manager.main.visiblePipeWirePage = 3;
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
        }
    }
}
