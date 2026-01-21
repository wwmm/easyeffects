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
import ee.pipewire as PW
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: pwPage

    readonly property bool showBackButton: columnView.currentIndex === 1 && columnView.columnResizeMode === Kirigami.ColumnView.SingleColumn

    function goBack(): void {
        columnView.currentIndex = 0;
    }

    padding: 0
    Component.onCompleted: {
        columnView.addItem(panelListView);

        panelModel.append({
            "title": i18n("General") // qmllint disable
            ,
            "iconName": "documentinfo-symbolic"
        });
        panelModel.append({
            "title": i18n("Modules") // qmllint disable
            ,
            "iconName": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Clients") // qmllint disable
            ,
            "iconName": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Test Signal") // qmllint disable
            ,
            "iconName": "waveform-symbolic"
        });
        panelListView.currentIndex = DbMain.visiblePipeWirePage;
        switch (DbMain.visiblePipeWirePage) {
        case 0:
            columnView.addItem(generalPage.createObject());
            break;
        case 1:
            columnView.addItem(modulesPage.createObject());
            break;
        case 2:
            columnView.addItem(clientsPage.createObject());
            break;
        case 3:
            columnView.addItem(testSignalPage.createObject());
            break;
        default:
            null;
        }
    }

    Component {
        id: generalPage

        FormCard.FormCardPage {
            id: generalFormCard

            function comboFindRow(model, nodeName) {
                let row = -1;
                const nodeIndex = PW.ModelNodes.getModelIndexByName(nodeName);
                const modelRow = model.mapFromSource(nodeIndex).row;
                if (modelRow >= 0)
                    row = modelRow;

                return row;
            }

            function updateInputDevComboSelection() {
                const deviceName = useDefaultInputDevice.isChecked ? PW.Manager.defaultInputDeviceName : DbStreamInputs.inputDevice;
                const comboRow = comboFindRow(PW.ModelSourceDevices, deviceName);
                if (comboRow !== -1)
                    comboInputDevice.currentIndex = comboRow;
            }

            function updateOutputDevComboSelection() {
                const deviceName = useDefaultOutputDevice.isChecked ? PW.Manager.defaultOutputDeviceName : DbStreamOutputs.outputDevice;
                const comboRow = comboFindRow(PW.ModelSinkDevices, deviceName);
                if (comboRow !== -1)
                    comboOutputDevice.currentIndex = comboRow;
            }

            Connections {
                function onDataChanged() {
                    generalFormCard.updateInputDevComboSelection();
                }

                target: PW.ModelSourceDevices
            }

            Connections {
                function onDataChanged() {
                    generalFormCard.updateOutputDevComboSelection();
                }

                target: PW.ModelSinkDevices
            }

            FormCard.FormHeader {
                title: i18n("Device Management") // qmllint disable
            }

            FormCard.FormSectionText {
                text: i18n("It's recommended to NOT set Easy Effects Sink/Source as Default Device in external applications (e.g. Gnome Settings and Plasma System Settings)") // qmllint disable
            }

            FormCard.FormCard {
                EeSwitch {
                    id: useDefaultInputDevice

                    label: i18n("Use default input") // qmllint disable
                    isChecked: DbStreamInputs.useDefaultInputDevice
                    onCheckedChanged: {
                        if (isChecked)
                            generalFormCard.updateInputDevComboSelection();

                        if (isChecked !== DbStreamInputs.useDefaultInputDevice)
                            DbStreamInputs.useDefaultInputDevice = isChecked;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboInputDevice

                    text: i18n("Name") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: PW.ModelSourceDevices
                    textRole: "description"
                    enabled: !DbStreamInputs.useDefaultInputDevice

                    onActivated: idx => {
                        const proxyIndex = PW.ModelSourceDevices.index(idx, 0);
                        const sourceIndex = PW.ModelSourceDevices.mapToSource(proxyIndex);
                        const nodeName = PW.ModelNodes.getNodeName(sourceIndex.row);
                        if (DbStreamInputs.inputDevice !== nodeName) {
                            if (!Common.isEmpty(nodeName))
                                DbStreamInputs.inputDevice = nodeName;
                        }
                    }

                    Component.onCompleted: {
                        generalFormCard.updateInputDevComboSelection();
                    }
                }
            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: useDefaultOutputDevice

                    label: i18n("Use default output") // qmllint disable
                    isChecked: DbStreamOutputs.useDefaultOutputDevice

                    onCheckedChanged: {
                        if (isChecked)
                            generalFormCard.updateOutputDevComboSelection();

                        if (isChecked !== DbStreamOutputs.useDefaultOutputDevice)
                            DbStreamOutputs.useDefaultOutputDevice = isChecked;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: comboOutputDevice

                    text: i18n("Name") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    editable: false
                    model: PW.ModelSinkDevices
                    textRole: "description"
                    enabled: !DbStreamOutputs.useDefaultOutputDevice

                    onActivated: idx => {
                        const proxyIndex = PW.ModelSinkDevices.index(idx, 0);
                        const sourceIndex = PW.ModelSinkDevices.mapToSource(proxyIndex);
                        const nodeName = PW.ModelNodes.getNodeName(sourceIndex.row);
                        if (DbStreamOutputs.outputDevice !== nodeName) {
                            if (!Common.isEmpty(nodeName))
                                DbStreamOutputs.outputDevice = nodeName;
                        }
                    }

                    Component.onCompleted: {
                        generalFormCard.updateOutputDevComboSelection();
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Server Information") // qmllint disable
            }

            FormCard.FormCard {
                FormCard.FormTextDelegate {
                    text: i18n("Header version") // qmllint disable
                    description: PW.Manager.headerVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Library version") // qmllint disable
                    description: PW.Manager.libraryVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Runtime version") // qmllint disable
                    description: PW.Manager.runtimeVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default sampling rate") // qmllint disable
                    description: `${PW.Manager.defaultClockRate} ${Units.hz}`
                }

                FormCard.FormTextDelegate {
                    text: i18n("Minimum quantum") // qmllint disable
                    description: PW.Manager.defaultMinQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Maximum quantum") // qmllint disable
                    description: PW.Manager.defaultMaxQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default quantum") // qmllint disable
                    description: PW.Manager.defaultQuantum
                }
            }
        }
    }

    Component {
        id: modulesPage

        Kirigami.ScrollablePage {
            ListView {
                id: modulesListView

                clip: true
                reuseItems: true
                model: PW.ModelModules

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: modulesListView.count === 0
                    text: i18n("No Modules") // qmllint disable
                }

                delegate: DelegateModulesList {}
            }
        }
    }

    Component {
        id: clientsPage

        Kirigami.ScrollablePage {
            ListView {
                id: clientsListView

                clip: true
                reuseItems: true
                model: PW.ModelClients

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: clientsListView.count === 0
                    text: i18n("No Clients") // qmllint disable
                }

                delegate: DelegateClientsList {}
            }
        }
    }

    Component {
        id: testSignalPage

        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("State") // qmllint disable
            }

            FormCard.FormCard {
                EeSwitch {
                    id: enableTestSignals

                    label: i18n("Enabled") // qmllint disable
                    isChecked: DbTestSignals.enable
                    onCheckedChanged: {
                        if (isChecked !== DbTestSignals.enable)
                            DbTestSignals.enable = isChecked;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Channels") // qmllint disable
            }

            FormCard.FormCard {
                enabled: enableTestSignals.isChecked

                FormCard.FormRadioDelegate {
                    id: leftChannel

                    text: i18n("Left") // qmllint disable
                    checked: DbTestSignals.channels === 0
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.channels)
                            DbTestSignals.channels = 0;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: rightChannel

                    text: i18n("Right") // qmllint disable
                    checked: DbTestSignals.channels === 1
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.channels)
                            DbTestSignals.channels = 1;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: bothChannels

                    text: i18n("Both") // qmllint disable
                    checked: DbTestSignals.channels === 2
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.channels)
                            DbTestSignals.channels = 2;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Waveform") // qmllint disable
            }

            FormCard.FormCard {
                enabled: enableTestSignals.isChecked

                FormCard.FormRadioDelegate {
                    id: sineWave

                    text: i18n("Sine wave") // qmllint disable
                    checked: DbTestSignals.signalType === 0
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.signalType)
                            DbTestSignals.signalType = 0;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: whiteNoise

                    text: i18n("White noise") // qmllint disable
                    checked: DbTestSignals.signalType === 1
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.signalType)
                            DbTestSignals.signalType = 1;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: pinkNoise

                    text: i18n("Pink noise") // qmllint disable
                    checked: DbTestSignals.signalType === 2
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.signalType)
                            DbTestSignals.signalType = 2;
                    }
                }

                FormCard.FormRadioDelegate {
                    id: silence

                    text: i18n("Silence") // qmllint disable
                    checked: DbTestSignals.signalType === 3
                    onCheckedChanged: {
                        if (checked !== DbTestSignals.signalType)
                            DbTestSignals.signalType = 3;
                    }
                }

                EeSpinBox {
                    id: frequency

                    label: i18n("Frequency") // qmllint disable
                    from: 10
                    to: 22000
                    value: DbTestSignals.frequency
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    enabled: sineWave.checked
                    onValueModified: v => {
                        if (v !== DbTestSignals.frequency)
                            DbTestSignals.frequency = v;
                    }
                }
            }
        }
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    contentItem: Kirigami.ColumnView {
        id: columnView

        columnResizeMode: root.width >= Kirigami.Units.gridUnit * 40 ? Kirigami.ColumnView.FixedColumns : Kirigami.ColumnView.SingleColumn
        columnWidth: Kirigami.Units.gridUnit * 15

        ListView {
            id: panelListView

            Layout.fillHeight: true
            Layout.minimumWidth: Math.round(pwPage.width * 0.15)
            clip: true
            reuseItems: true

            model: ListModel {
                id: panelModel
            }

            delegate: Delegates.RoundedItemDelegate {
                id: listItemDelegate

                property int elide: Text.ElideRight
                required property int index
                required property string title
                required property string iconName

                highlighted: ListView.isCurrentItem
                onClicked: {
                    ListView.view.currentIndex = index;
                    switch (index) {
                    case 0:
                        columnView.replaceItem(1, generalPage.createObject());
                        DbMain.visiblePipeWirePage = 0;
                        break;
                    case 1:
                        columnView.replaceItem(1, modulesPage.createObject());
                        DbMain.visiblePipeWirePage = 1;
                        break;
                    case 2:
                        columnView.replaceItem(1, clientsPage.createObject());
                        DbMain.visiblePipeWirePage = 2;
                        break;
                    case 3:
                        columnView.replaceItem(1, testSignalPage.createObject());
                        DbMain.visiblePipeWirePage = 3;
                        break;
                    default:
                        console.log("pipewire page stackview: invalid index");
                    }
                    columnView.currentIndex = 1;
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: listItemDelegate.iconName
                        Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.alignment: Qt.AlignLeft
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: listItemDelegate.title
                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }
}
