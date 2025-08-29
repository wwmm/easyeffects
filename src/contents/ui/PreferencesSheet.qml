import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQml // Despite of what Qt extension says this import is needed. We crash without it
import ee.database as DB
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.OverlaySheet {
    id: preferencesSheet

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    title: i18n("Preferences")
    y: appWindow.header.height + Kirigami.Units.gridUnit
    implicitWidth: Math.min(stack.implicitWidth, appWindow.width * 0.8)
    implicitHeight: Math.min(2 * preferencesSheet.header.height + stack.implicitHeight, preferencesSheet.parent.height - 2 * preferencesSheet.header.height - preferencesSheet.y)
    onVisibleChanged: {
        if (!preferencesSheet.visible) {
            while (stack.depth > 1)
                stack.pop();
            headerTitle.text = i18n("Preferences");
        }
    }

    Component {
        id: servicePage

        ColumnLayout {
            FormCard.FormCard {
                EeSwitch {
                    id: enableServiceMode

                    label: i18n("Enable Service Mode")
                    isChecked: DB.Manager.main.enableServiceMode
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.enableServiceMode)
                            DB.Manager.main.enableServiceMode = isChecked;
                    }
                }

                EeSwitch {
                    id: autostartOnLogin

                    label: i18n("Autostart on Login")
                    isChecked: DB.Manager.main.autostartOnLogin
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.autostartOnLogin) {
                            DB.Manager.main.autostartOnLogin = isChecked;
                        }
                    }
                }

                EeSwitch {
                    id: showTrayIcon

                    label: i18n("Show the Tray Icon")
                    isChecked: DB.Manager.main.showTrayIcon && canUseSysTray
                    enabled: canUseSysTray
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.showTrayIcon)
                            DB.Manager.main.showTrayIcon = isChecked;
                    }
                }
            }
        }
    }

    Component {
        id: audioPage

        ColumnLayout {
            FormCard.FormCard {
                EeSwitch {
                    id: processAllOutputs

                    label: i18n("Process All Output Streams")
                    isChecked: DB.Manager.main.processAllOutputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllOutputs)
                            DB.Manager.main.processAllOutputs = isChecked;
                    }
                }

                EeSwitch {
                    id: processAllInputs

                    label: i18n("Process All Input Streams")
                    isChecked: DB.Manager.main.processAllInputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllInputs)
                            DB.Manager.main.processAllInputs = isChecked;
                    }
                }

                EeSwitch {
                    id: excludeMonitorStreams

                    label: i18n("Ignore Streams from Monitor of Devices")
                    isChecked: DB.Manager.main.excludeMonitorStreams
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.excludeMonitorStreams)
                            DB.Manager.main.excludeMonitorStreams = isChecked;
                    }
                }

                EeSwitch {
                    id: useCubicVolumes

                    label: i18n("Use Cubic Volume")
                    isChecked: DB.Manager.main.useCubicVolumes
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.useCubicVolumes)
                            DB.Manager.main.useCubicVolumes = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMic

                    label: i18n("Enable/disable input monitoring")
                    isChecked: DB.Manager.streamInputs.listenToMic
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMic)
                            DB.Manager.streamInputs.listenToMic = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMicIncludesOutputEffects

                    label: i18n("Mic Monitor Plays to Output Effects Pipeline")
                    isChecked: DB.Manager.streamInputs.listenToMicIncludesOutputEffects
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMicIncludesOutputEffects)
                            DB.Manager.streamInputs.listenToMicIncludesOutputEffects = isChecked;
                    }
                }

                EeSwitch {
                    id: inactivityTimerEnable

                    label: i18n("Enable the Inactivity Timeout")
                    isChecked: DB.Manager.main.inactivityTimerEnable
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.inactivityTimerEnable)
                            DB.Manager.main.inactivityTimerEnable = isChecked;
                    }
                }

                EeSpinBox {
                    id: inactivityTimeout

                    label: i18n("Inactivity Timeout")
                    from: DB.Manager.main.getMinValue("inactivityTimeout")
                    to: DB.Manager.main.getMaxValue("inactivityTimeout")
                    value: DB.Manager.main.inactivityTimeout
                    decimals: 0
                    stepSize: 1
                    unit: "s"
                    enabled: DB.Manager.main.inactivityTimerEnable
                    onValueModified: v => {
                        DB.Manager.main.inactivityTimeout = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Level Meters Label")
                    from: DB.Manager.main.getMinValue("levelMetersLabelTimer")
                    to: DB.Manager.main.getMaxValue("levelMetersLabelTimer")
                    value: DB.Manager.main.levelMetersLabelTimer
                    decimals: 0
                    stepSize: 1
                    unit: "ms"
                    enabled: DB.Manager.main.levelMetersLabelTimer
                    onValueModified: v => {
                        DB.Manager.main.levelMetersLabelTimer = v;
                    }
                }
            }
        }
    }

    Component {
        id: spectrumPage

        ColumnLayout {
            FormCard.FormHeader {
                title: i18n("State")
            }

            FormCard.FormCard {
                EeSwitch {
                    id: spectrumState

                    label: i18n("Enabled")
                    isChecked: DB.Manager.spectrum.state
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.state)
                            DB.Manager.spectrum.state = isChecked;
                    }
                }

                EeSpinBox {
                    id: avsyncDelay

                    label: i18n("Audio Delay Compensation")
                    from: 0
                    to: 1000
                    value: DB.Manager.spectrum.avsyncDelay
                    decimals: 0
                    stepSize: 1
                    unit: "ms"
                    onValueModified: v => {
                        DB.Manager.spectrum.avsyncDelay = v;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Style")
            }

            FormCard.FormCard {
                FormCard.FormComboBoxDelegate {
                    id: spectrumColorScheme

                    text: i18n("Color Scheme")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumColorScheme
                    editable: false
                    model: [i18n("Automatic"), i18n("Light"), i18n("Dark")]
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumColorScheme)
                            DB.Manager.spectrum.spectrumColorScheme = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: spectrumColorTheme

                    text: i18n("Color Theme")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumColorTheme
                    editable: false
                    model: [i18n("Green"), i18n("Green Neon"), i18n("Mix"), i18n("Orange"), i18n("Yellow"), i18n("Blue"), i18n("Purple"), i18n("Grey")]
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumColorTheme)
                            DB.Manager.spectrum.spectrumColorTheme = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: spectrumShape

                    text: i18n("Shape")
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumShape
                    editable: false
                    model: [i18n("Bars"), i18n("Lines"), i18n("Dots"), i18n("Area")]
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumShape)
                            DB.Manager.spectrum.spectrumShape = idx;
                    }
                }

                EeSwitch {
                    id: dynamicYScale

                    label: i18n("Dynamic Scale")
                    isChecked: DB.Manager.spectrum.dynamicYScale
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.dynamicYScale)
                            DB.Manager.spectrum.dynamicYScale = isChecked;
                    }
                }

                EeSwitch {
                    id: logarithimicHorizontalAxis

                    label: i18n("Logarithmic Frequency Axis")
                    isChecked: DB.Manager.spectrum.logarithimicHorizontalAxis
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.logarithimicHorizontalAxis)
                            DB.Manager.spectrum.logarithimicHorizontalAxis = isChecked;
                    }
                }

                EeSpinBox {
                    id: nPoints

                    label: i18n("Points")
                    from: 2
                    to: 2048
                    value: DB.Manager.spectrum.nPoints
                    decimals: 0
                    stepSize: 1
                    onValueModified: v => {
                        DB.Manager.spectrum.nPoints = v;
                    }
                }

                EeSpinBox {
                    id: height

                    label: i18n("Height")
                    from: 100
                    to: 1000
                    value: DB.Manager.spectrum.height
                    decimals: 0
                    stepSize: 1
                    unit: "px"
                    onValueModified: v => {
                        DB.Manager.spectrum.height = v;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Frequency Range")
            }

            FormCard.FormCard {
                EeSpinBox {
                    id: minimumFrequency

                    label: i18n("Minimum")
                    from: 20
                    to: 21900
                    value: DB.Manager.spectrum.minimumFrequency
                    decimals: 0
                    stepSize: 10
                    unit: "Hz"
                    onValueModified: v => {
                        DB.Manager.spectrum.minimumFrequency = v;
                    }
                }

                EeSpinBox {
                    id: maximumFrequency

                    label: i18n("Maximum")
                    from: 120
                    to: 22000
                    value: DB.Manager.spectrum.maximumFrequency
                    decimals: 0
                    stepSize: 10
                    unit: "Hz"
                    onValueModified: v => {
                        DB.Manager.spectrum.maximumFrequency = v;
                    }
                }
            }
        }
    }

    Component {
        id: databasePage

        ColumnLayout {
            FormCard.FormCard {
                EeSpinBox {
                    label: i18n("Database Autosave Interval")
                    from: DB.Manager.main.getMinValue("databaseAutosaveInterval")
                    to: DB.Manager.main.getMaxValue("databaseAutosaveInterval")
                    value: DB.Manager.main.databaseAutosaveInterval
                    decimals: 0
                    stepSize: 1
                    unit: "ms"
                    onValueModified: v => {
                        DB.Manager.main.databaseAutosaveInterval = v;
                    }
                }
            }
        }
    }

    Component {
        id: experimentalPage

        ColumnLayout {
            FormCard.FormCard {
                EeSwitch {
                    id: xdgGlobalShortcuts

                    label: i18n("Global Shortcuts")
                    subtitle: i18n("Enables support for XDG Global Shortcuts")
                    isChecked: DB.Manager.main.xdgGlobalShortcuts
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.xdgGlobalShortcuts)
                            DB.Manager.main.xdgGlobalShortcuts = isChecked;
                    }
                }

                EeSwitch {
                    id: showNativePluginUi

                    label: i18n("Native Window of Effects")
                    subtitle: i18n("Allows the Native User Interface of Effects to be Shown/Hidden")
                    isChecked: DB.Manager.main.showNativePluginUi
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.showNativePluginUi)
                            DB.Manager.main.showNativePluginUi = isChecked;
                    }
                }

                EeSpinBox {
                    id: lv2uiUpdateFrequency

                    label: i18n("Update Frequency")
                    subtitle: i18n("Related to LV2 Plugins")
                    from: 1
                    to: 60
                    value: DB.Manager.main.lv2uiUpdateFrequency
                    decimals: 0
                    stepSize: 1
                    unit: "Hz"
                    enabled: DB.Manager.main.showNativePluginUi
                    onValueModified: v => {
                        DB.Manager.main.lv2uiUpdateFrequency = v;
                    }
                }
            }
        }
    }

    Controls.StackView {
        id: stack

        implicitWidth: Kirigami.Units.gridUnit * 30
        implicitHeight: stack.currentItem.implicitHeight

        initialItem: ColumnLayout {
            FormCard.FormCard {
                FormCard.FormButtonDelegate {
                    id: serviceButton

                    icon.name: "services-symbolic"
                    text: i18n("Background Service")
                    onClicked: {
                        while (stack.depth > 1)
                            stack.pop();
                        stack.push(servicePage);
                        headerTitle.text = text;
                    }
                }

                FormCard.FormButtonDelegate {
                    id: audioButton

                    icon.name: "folder-sound-symbolic"
                    text: i18n("Audio")
                    onClicked: {
                        while (stack.depth > 1)
                            stack.pop();
                        stack.push(audioPage);
                        headerTitle.text = text;
                    }
                }

                FormCard.FormButtonDelegate {
                    id: spectrumButton

                    icon.name: "folder-chart-symbolic"
                    text: i18n("Spectrum Analyzer")
                    onClicked: {
                        while (stack.depth > 1)
                            stack.pop();
                        stack.push(spectrumPage);
                        headerTitle.text = text;
                    }
                }

                FormCard.FormButtonDelegate {
                    id: databaseButton

                    icon.name: "server-database-symbolic"
                    text: i18n("Database")
                    onClicked: {
                        while (stack.depth > 1)
                            stack.pop();
                        stack.push(databasePage);
                        headerTitle.text = text;
                    }
                }

                FormCard.FormButtonDelegate {
                    id: experimentalButton

                    icon.name: "emblem-warning"
                    text: i18n("Experimental Features")
                    onClicked: {
                        while (stack.depth > 1)
                            stack.pop();
                        stack.push(experimentalPage);
                        headerTitle.text = text;
                    }
                }
            }
        }
    }

    header: RowLayout {
        Controls.ToolButton {
            id: headerBackButton

            icon.name: "draw-arrow-back"
            visible: stack.depth !== 1
            onClicked: {
                while (stack.depth > 1)
                    stack.pop();
                headerTitle.text = i18n("Preferences");
            }
        }

        Kirigami.Icon {
            visible: stack.depth === 1
            source: "gtk-preferences-symbolic"
        }

        Kirigami.Heading {
            id: headerTitle

            Layout.fillWidth: true
            text: i18n("Preferences")
        }
    }
}
