//pragma explanation: https://doc.qt.io/qt-6/qtqml-documents-structure.html
pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.tags.plugin.name as TagsPluginName
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.ScrollablePage {
    id: equalizerPage

    required property var name
    required property var pluginDB
    required property var leftDB
    required property var rightDB
    required property var pipelineInstance
    property var pluginBackend

    function updateMeters() {
        if (!pluginBackend)
            return;

        inputOutputLevels.inputLevelLeft = pluginBackend.getInputLevelLeft();
        inputOutputLevels.inputLevelRight = pluginBackend.getInputLevelRight();
        inputOutputLevels.outputLevelLeft = pluginBackend.getOutputLevelLeft();
        inputOutputLevels.outputLevelRight = pluginBackend.getOutputLevelRight();
    }

    Component.onCompleted: {
        pluginBackend = pipelineInstance.getPluginInstance(name);
    }

    Controls.Popup {
        id: bandMenu

        property int index: 0
        property var menuButton: null
        readonly property var bandDB: {
            equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? equalizerPage.leftDB : equalizerPage.rightDB) : equalizerPage.leftDB;
        }

        parent: menuButton
        focus: true
        x: parent !== null ? Math.round((parent.width - width) / 2) : x
        y: parent !== null ? parent.height : y
        closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
        onClosed: {
            menuButton.checked = false;
        }

        contentItem: ColumnLayout {
            Kirigami.CardsLayout {
                Kirigami.Card {
                    actions: [
                        Kirigami.Action {
                            readonly property string bandName: "band" + bandMenu.index + "Mute"
                            text: i18n("Mute")
                            icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-low-symbolic"
                            checkable: true
                            checked: bandMenu.bandDB[bandName]
                            onTriggered: {
                                if (checked != bandMenu.bandDB[bandName])
                                    bandMenu.bandDB[bandName] = checked;
                            }
                        },
                        Kirigami.Action {
                            readonly property string bandName: "band" + bandMenu.index + "Solo"
                            text: i18n("Solo")
                            checkable: true
                            icon.name: "starred-symbolic"
                            checked: bandMenu.bandDB[bandName]
                            onTriggered: {
                                if (checked != bandMenu.bandDB[bandName])
                                    bandMenu.bandDB[bandName] = checked;
                            }
                        }
                    ]
                    contentItem: GridLayout {
                        uniformCellWidths: true
                        rowSpacing: Kirigami.Units.largeSpacing
                        columnSpacing: Kirigami.Units.largeSpacing
                        columns: 2
                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + bandMenu.index + "Type"
                            text: i18n("Type")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandMenu.bandDB[bandName]
                            editable: false
                            model: [i18n("Off"), i18n("Bell"), i18n("Hi-pass"), i18n("Hi-shelf"), i18n("Lo-pass"), i18n("Lo-shelf"), i18n("Notch"), i18n("Resonance"), i18n("Allpass"), i18n("Bandpass"), i18n("Ladder-pass"), i18n("Ladder-rej")]
                            onActivated: idx => {
                                bandMenu.bandDB[bandName] = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + bandMenu.index + "Mode"
                            text: i18n("Mode")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandMenu.bandDB[bandName]
                            editable: false
                            model: [i18n("RLC (BT)"), i18n("RLC (MT)"), i18n("BWC (BT)"), i18n("BWC (MT)"), i18n("LRX (BT)"), i18n("LRX (MT)"), i18n("APO (DR)")]
                            onActivated: idx => {
                                bandMenu.bandDB[bandName] = idx;
                            }
                        }

                        FormCard.FormComboBoxDelegate {
                            readonly property string bandName: "band" + bandMenu.index + "Slope"
                            text: i18n("Slope")
                            displayMode: FormCard.FormComboBoxDelegate.ComboBox
                            currentIndex: bandMenu.bandDB[bandName]
                            editable: false
                            model: [i18n("x1"), i18n("x2"), i18n("x3"), i18n("x4")]
                            onActivated: idx => {
                                bandMenu.bandDB[bandName] = idx;
                            }
                        }

                        EeSpinBox {
                            readonly property string bandName: "band" + bandMenu.index + "Frequency"
                            label: i18n("Frequency")
                            labelAbove: true
                            spinboxLayoutFillWidth: true
                            from: bandMenu.bandDB.getMinValue(bandName)
                            to: bandMenu.bandDB.getMaxValue(bandName)
                            value: bandMenu.bandDB[bandName]
                            decimals: 0
                            stepSize: 1
                            unit: "Hz"
                            onValueModified: v => {
                                bandMenu.bandDB[bandName] = v;
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        height: equalizerPage.height - equalizerPage.header.height - equalizerPage.footer.height - Kirigami.Units.gridUnit
        Kirigami.CardsLayout {
            maximumColumns: 5
            readonly property real columnSize: pitchLeft.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize
            Layout.fillHeight: false

            FormCard.FormComboBoxDelegate {
                id: mode

                text: i18n("Mode")
                displayMode: FormCard.FormComboBoxDelegate.ComboBox
                currentIndex: pluginDB.mode
                editable: false
                model: [i18n("IIR"), i18n("FIR"), i18n("FFT"), i18n("SPM")]
                onActivated: idx => {
                    pluginDB.mode = idx;
                }
            }

            EeSpinBox {
                id: numBands

                label: i18n("Bands")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("numBands")
                to: pluginDB.getMaxValue("numBands")
                value: pluginDB.numBands
                decimals: 0
                stepSize: 1
                onValueModified: v => {
                    pluginDB.numBands = v;
                }
            }

            EeSpinBox {
                id: balance

                label: i18n("Balance")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("balance")
                to: pluginDB.getMaxValue("balance")
                value: pluginDB.balance
                decimals: 1
                stepSize: 0.1
                unit: "%"
                onValueModified: v => {
                    pluginDB.balance = v;
                }
            }

            EeSpinBox {
                id: pitchLeft

                label: i18n("Pitch Left")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("pitchLeft")
                to: pluginDB.getMaxValue("pitchLeft")
                value: pluginDB.pitchLeft
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    pluginDB.pitchLeft = v;
                }
            }

            EeSpinBox {
                id: pitchRight

                label: i18n("Pitch Right")
                labelAbove: true
                spinboxLayoutFillWidth: true
                from: pluginDB.getMinValue("pitchRight")
                to: pluginDB.getMaxValue("pitchRight")
                value: pluginDB.pitchRight
                decimals: 2
                stepSize: 0.01
                unit: "st"
                onValueModified: v => {
                    pluginDB.pitchRight = v;
                }
            }
        }

        Kirigami.CardsLayout {
            maximumColumns: 1
            readonly property real columnSize: bandsCard.implicitWidth
            minimumColumnWidth: columnSize
            maximumColumnWidth: columnSize
            Layout.minimumHeight: Kirigami.Units.gridUnit * 20

            Kirigami.Card {
                id: bandsCard
                Layout.fillHeight: true
                actions: [
                    Kirigami.Action {
                        id: viewLeft
                        visible: equalizerPage.pluginDB.splitChannels
                        checkable: true
                        checked: equalizerPage.pluginDB.viewLeftChannel
                        icon.name: "arrow-left-symbolic"
                        onTriggered: {
                            equalizerPage.pluginDB.viewLeftChannel = true;
                        }
                    },
                    Kirigami.Action {
                        id: viewRight
                        visible: equalizerPage.pluginDB.splitChannels
                        checkable: true
                        checked: !equalizerPage.pluginDB.viewLeftChannel
                        icon.name: "arrow-right-symbolic"
                        onTriggered: {
                            equalizerPage.pluginDB.viewLeftChannel = false;
                        }
                    }
                ]

                header: Kirigami.Heading {
                    visible: equalizerPage.pluginDB.splitChannels
                    text: equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? i18n("Left") : i18n("Right")) : ""
                    level: 2
                }

                footer: Controls.ScrollBar {
                    id: listViewScrollBar

                    Layout.fillWidth: true
                }

                contentItem: ListView {
                    id: listview

                    implicitWidth: contentItem.childrenRect.width < equalizerPage.width ? contentItem.childrenRect.width : equalizerPage.width - 4 * (bandsCard.leftPadding + bandsCard.rightPadding)
                    clip: true
                    reuseItems: true
                    orientation: ListView.Horizontal
                    model: equalizerPage.pluginDB.numBands
                    Controls.ScrollBar.horizontal: listViewScrollBar

                    delegate: EqualizerBand {
                        bandDB: {
                            equalizerPage.pluginDB.splitChannels ? (equalizerPage.pluginDB.viewLeftChannel ? equalizerPage.leftDB : equalizerPage.rightDB) : equalizerPage.leftDB;
                        }
                        menu: bandMenu
                    }
                }
            }
        }
    }

    header: EeInputOutputGain {
        id: inputOutputLevels

        pluginDB: equalizerPage.pluginDB
    }

    footer: RowLayout {
        Controls.Label {
            text: i18n("Using %1", `<b>${TagsPluginName.Package.lsp}</b>`)
            textFormat: Text.RichText
            horizontalAlignment: Qt.AlignLeft
            verticalAlignment: Qt.AlignVCenter
            Layout.fillWidth: false
            Layout.leftMargin: Kirigami.Units.smallSpacing
            color: Kirigami.Theme.disabledTextColor
        }

        Kirigami.ActionToolBar {
            Layout.margins: Kirigami.Units.smallSpacing
            alignment: Qt.AlignRight
            position: Controls.ToolBar.Footer
            flat: true
            actions: [
                Kirigami.Action {
                    text: i18n("Show Native Window")
                    icon.name: "window-duplicate-symbolic"
                    enabled: DB.Manager.main.showNativePluginUi
                    checkable: true
                    checked: pluginBackend.hasNativeUi()
                    onTriggered: {
                        if (checked)
                            pluginBackend.showNativeUi();
                        else
                            pluginBackend.closeNativeUi();
                    }
                },
                Kirigami.Action {
                    text: i18n("Split Channels")
                    icon.name: "split-symbolic"
                    checkable: true
                    checked: pluginDB.splitChannels
                    onTriggered: {
                        if (pluginDB.splitChannels != checked)
                            pluginDB.splitChannels = checked;
                    }
                },
                Kirigami.Action {
                    text: i18n("Flat Response")
                    icon.name: "map-flat-symbolic"
                    onTriggered: {
                        pluginBackend.flatResponse();
                    }
                },
                Kirigami.Action {
                    text: i18n("Calculate Frequencies")
                    icon.name: "folder-calculate-symbolic"
                    onTriggered: {
                        pluginBackend.calculateFrequencies();
                    }
                },
                Kirigami.Action {
                    text: i18n("Sort Bands")
                    icon.name: "sort_incr-symbolic"
                    onTriggered: {
                        pluginBackend.sortBands();
                    }
                },
                Kirigami.Action {
                    text: i18n("Reset")
                    icon.name: "edit-reset-symbolic"
                    onTriggered: {
                        pluginBackend.reset();
                    }
                }
            ]
        }
    }
}
