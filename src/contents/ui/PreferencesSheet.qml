import QtQuick.Controls as Controls
import QtQuick.Layouts
import QtQml // Despite of what Qt extension says this import is needed. We crash without it
import ee.database as DB
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.OverlaySheet {
    id: preferencesSheet

    parent: applicationWindow().overlay// qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    title: i18n("Preferences") // qmllint disable
    y: 0
    implicitWidth: Math.min(stack.implicitWidth, appWindow.width * 0.8)// qmllint disable
    implicitHeight: Math.min(2 * preferencesSheet.header.height + stack.implicitHeight, preferencesSheet.parent.height - 2 * preferencesSheet.header.height)
    onVisibleChanged: {
        if (!preferencesSheet.visible) {
            while (stack.depth > 1)
                stack.pop();
            headerTitle.text = i18n("Preferences");// qmllint disable
        }
    }

    Component {
        id: servicePage

        ColumnLayout {
            FormCard.FormCard {
                EeSwitch {
                    id: enableServiceMode

                    label: i18n("Enable Service Mode") // qmllint disable
                    subtitle: i18n("Easy Effects is Active in Background When the Window is Closed") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.enableServiceMode
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.enableServiceMode)
                            DB.Manager.main.enableServiceMode = isChecked;
                    }
                }

                EeSwitch {
                    id: autostartOnLogin

                    label: i18n("Autostart on Login") // qmllint disable
                    subtitle: i18n("Easy Effects is Launched at User Session Startup") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.autostartOnLogin
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.autostartOnLogin) {
                            DB.Manager.main.autostartOnLogin = isChecked;
                        }
                    }
                }

                EeSwitch {
                    id: showTrayIcon

                    label: i18n("Show the Tray Icon") // qmllint disable
                    isChecked: DB.Manager.main.showTrayIcon && canUseSysTray// qmllint disable
                    enabled: canUseSysTray// qmllint disable
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

                    label: i18n("Process All Output Streams") // qmllint disable
                    isChecked: DB.Manager.main.processAllOutputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllOutputs)
                            DB.Manager.main.processAllOutputs = isChecked;
                    }
                }

                EeSwitch {
                    id: processAllInputs

                    label: i18n("Process All Input Streams") // qmllint disable
                    isChecked: DB.Manager.main.processAllInputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllInputs)
                            DB.Manager.main.processAllInputs = isChecked;
                    }
                }

                EeSwitch {
                    id: excludeMonitorStreams

                    label: i18n("Ignore Streams from Monitor of Devices") // qmllint disable
                    isChecked: DB.Manager.main.excludeMonitorStreams
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.excludeMonitorStreams)
                            DB.Manager.main.excludeMonitorStreams = isChecked;
                    }
                }

                EeSwitch {
                    id: useCubicVolumes

                    label: i18n("Use Cubic Volume") // qmllint disable
                    isChecked: DB.Manager.main.useCubicVolumes
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.useCubicVolumes)
                            DB.Manager.main.useCubicVolumes = isChecked;
                    }
                }

                EeSwitch {
                    id: ignoreSystemNotifications

                    label: i18n("Ignore System Notifications") // qmllint disable
                    subtitle: i18n("Processing System Notifications May Cause Crackling Sound, so It Would be Better to Ignore Them, but For Some Presets It May be Recommended Since They Could Sound Too Loud") // qmllint disable
                    isChecked: DB.Manager.main.ignoreSystemNotifications
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.ignoreSystemNotifications)
                            DB.Manager.main.ignoreSystemNotifications = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMic

                    label: i18n("Enable/Disable Input Monitoring") // qmllint disable
                    isChecked: DB.Manager.streamInputs.listenToMic
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMic)
                            DB.Manager.streamInputs.listenToMic = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMicIncludesOutputEffects

                    label: i18n("Mic Monitor Plays to Output Effects Pipeline") // qmllint disable
                    isChecked: DB.Manager.streamInputs.listenToMicIncludesOutputEffects
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMicIncludesOutputEffects)
                            DB.Manager.streamInputs.listenToMicIncludesOutputEffects = isChecked;
                    }
                }

                EeSwitch {
                    id: inactivityTimerEnable

                    label: i18n("Enable the Inactivity Timeout") // qmllint disable
                    subtitle: i18n("When all Streams are Inactive, Easy Effects Pipeline Remains Loaded for an Extra Amount of Time") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.inactivityTimerEnable
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.inactivityTimerEnable)
                            DB.Manager.main.inactivityTimerEnable = isChecked;
                    }
                }

                EeSpinBox {
                    id: inactivityTimeout

                    label: i18n("Inactivity Timeout") // qmllint disable
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
                    label: i18n("Level Meters Label") // qmllint disable
                    subtitle: i18n("The Time it Takes for the Level Meter Labels to be Updated When the Current Level is Below the Last Highest One") // qmllint disable
                    maximumLineCount: -1
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
                title: i18n("State") // qmllint disable
            }

            FormCard.FormCard {
                EeSwitch {
                    id: spectrumState

                    label: i18n("Enabled") // qmllint disable
                    isChecked: DB.Manager.spectrum.state
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.state)
                            DB.Manager.spectrum.state = isChecked;
                    }
                }

                EeSpinBox {
                    id: avsyncDelay

                    label: i18n("Audio Delay Compensation") // qmllint disable
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
                title: i18n("Style") // qmllint disable
            }

            FormCard.FormCard {
                FormCard.FormComboBoxDelegate {
                    id: spectrumColorScheme

                    text: i18n("Color Scheme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumColorScheme
                    editable: false
                    model: [i18n("Automatic"), i18n("Light"), i18n("Dark")]// qmllint disable
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumColorScheme)
                            DB.Manager.spectrum.spectrumColorScheme = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: spectrumColorTheme

                    text: i18n("Color Theme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumColorTheme
                    editable: false
                    model: [i18n("Green"), i18n("Green Neon"), i18n("Mix"), i18n("Orange"), i18n("Yellow"), i18n("Blue"), i18n("Purple"), i18n("Grey")]// qmllint disable
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumColorTheme)
                            DB.Manager.spectrum.spectrumColorTheme = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: spectrumShape

                    text: i18n("Shape") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumShape
                    editable: false
                    model: [i18n("Bars"), i18n("Lines"), i18n("Dots"), i18n("Area")]// qmllint disable
                    onActivated: idx => {
                        if (idx !== DB.Manager.spectrum.spectrumShape)
                            DB.Manager.spectrum.spectrumShape = idx;
                    }
                }

                EeSwitch {
                    id: dynamicYScale

                    label: i18n("Dynamic Scale") // qmllint disable
                    isChecked: DB.Manager.spectrum.dynamicYScale
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.dynamicYScale)
                            DB.Manager.spectrum.dynamicYScale = isChecked;
                    }
                }

                EeSwitch {
                    id: logarithimicHorizontalAxis

                    label: i18n("Logarithmic Frequency Axis") // qmllint disable
                    isChecked: DB.Manager.spectrum.logarithimicHorizontalAxis
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.logarithimicHorizontalAxis)
                            DB.Manager.spectrum.logarithimicHorizontalAxis = isChecked;
                    }
                }

                EeSpinBox {
                    id: nPoints

                    label: i18n("Points") // qmllint disable
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

                    label: i18n("Height") // qmllint disable
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
                title: i18n("Frequency Range") // qmllint disable
            }

            FormCard.FormCard {
                EeSpinBox {
                    id: minimumFrequency

                    label: i18n("Minimum") // qmllint disable
                    from: DB.Manager.spectrum.getMinValue("minimumFrequency")
                    to: DB.Manager.spectrum.getMaxValue("minimumFrequency")
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

                    label: i18n("Maximum") // qmllint disable
                    from: DB.Manager.spectrum.getMinValue("maximumFrequency")
                    to: DB.Manager.spectrum.getMaxValue("maximumFrequency")
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
                    label: i18n("Database Autosave Interval") // qmllint disable
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

                    label: i18n("Global Shortcuts") // qmllint disable
                    subtitle: i18n("Enables Support for XDG Global Shortcuts") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.xdgGlobalShortcuts
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.xdgGlobalShortcuts)
                            DB.Manager.main.xdgGlobalShortcuts = isChecked;
                    }
                }

                EeSwitch {
                    id: showNativePluginUi

                    label: i18n("Native Window of Effects") // qmllint disable
                    subtitle: i18n("Allows the Native User Interface of Effects to be Shown/Hidden") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.showNativePluginUi
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.showNativePluginUi)
                            DB.Manager.main.showNativePluginUi = isChecked;
                    }
                }

                EeSpinBox {
                    id: lv2uiUpdateFrequency

                    label: i18n("Update Frequency") // qmllint disable
                    subtitle: i18n("Related to LV2 Plugins") // qmllint disable
                    maximumLineCount: -1
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
                    text: i18n("Background Service") // qmllint disable
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
                    text: i18n("Audio") // qmllint disable
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
                    text: i18n("Spectrum Analyzer") // qmllint disable
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
                    text: i18n("Database") // qmllint disable
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
                    text: i18n("Experimental Features") // qmllint disable
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
                headerTitle.text = i18n("Preferences");// qmllint disable
            }
        }

        Kirigami.Icon {
            visible: stack.depth === 1
            source: "gtk-preferences-symbolic"
        }

        Kirigami.Heading {
            id: headerTitle

            Layout.fillWidth: true
            text: i18n("Preferences") // qmllint disable
        }
    }
}
