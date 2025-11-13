import QtQml // Despite of what Qt extension says this import is needed. We crash without it
import QtQuick
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationView {
    id: preferencesSheet

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "appearance"
            icon.name: "services-symbolic"
            text: i18n("Background Service") // qmllint disable
            page: () => servicePage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "audio"
            icon.name: "folder-sound-symbolic"
            text: i18n("Audio") // qmllint disable
            page: () => audioPage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "spectrum"
            icon.name: "folder-chart-symbolic"
            text: i18n("Spectrum Analyzer") // qmllint disable
            page: () => spectrumPage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "database"
            icon.name: "server-database-symbolic"
            text: i18n("Database") // qmllint disable
            page: () => databasePage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "style"
            icon.name: "preferences-desktop-theme-global-symbolic"
            text: i18n("Style") // qmllint disable
            page: () => stylePage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "experimental"
            icon.name: "emblem-warning"
            text: i18n("Experimental Features") // qmllint disable
            page: () => experimentalPage
        }
    ]

    readonly property Component servicePage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: enableServiceMode

                    label: i18n("Enable service mode") // qmllint disable
                    subtitle: i18n("Easy Effects is active in background when the window is closed.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.enableServiceMode
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.enableServiceMode)
                            DB.Manager.main.enableServiceMode = isChecked;
                    }
                }

                EeSwitch {
                    id: autostartOnLogin

                    label: i18n("Autostart on login") // qmllint disable
                    subtitle: i18n("Easy Effects is launched at user session startup.") // qmllint disable
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

                    label: i18n("Show the tray icon") // qmllint disable
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

    readonly property Component audioPage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: processAllOutputs

                    label: i18n("Process all output streams") // qmllint disable
                    isChecked: DB.Manager.main.processAllOutputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllOutputs)
                            DB.Manager.main.processAllOutputs = isChecked;
                    }
                }

                EeSwitch {
                    id: processAllInputs

                    label: i18n("Process all input streams") // qmllint disable
                    isChecked: DB.Manager.main.processAllInputs
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.processAllInputs)
                            DB.Manager.main.processAllInputs = isChecked;
                    }
                }

                EeSwitch {
                    id: excludeMonitorStreams

                    label: i18n("Ignore streams from monitor of devices") // qmllint disable
                    isChecked: DB.Manager.main.excludeMonitorStreams
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.excludeMonitorStreams)
                            DB.Manager.main.excludeMonitorStreams = isChecked;
                    }
                }

                EeSwitch {
                    id: useCubicVolumes

                    label: i18n("Use cubic volume") // qmllint disable
                    isChecked: DB.Manager.main.useCubicVolumes
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.useCubicVolumes)
                            DB.Manager.main.useCubicVolumes = isChecked;
                    }
                }

                EeSwitch {
                    id: ignoreSystemNotifications

                    label: i18n("Ignore system notifications") // qmllint disable
                    subtitle: i18n("Processing system notifications may cause crackling sound, so it would be better to ignore them, but for some presets it may be recommended since they could sound too loud.") // qmllint disable
                    isChecked: DB.Manager.main.ignoreSystemNotifications
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.ignoreSystemNotifications)
                            DB.Manager.main.ignoreSystemNotifications = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMic

                    label: i18n("Enable/disable input monitoring") // qmllint disable
                    isChecked: DB.Manager.streamInputs.listenToMic
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMic)
                            DB.Manager.streamInputs.listenToMic = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMicIncludesOutputEffects

                    label: i18n("Mic monitor plays to output effects pipeline") // qmllint disable
                    isChecked: DB.Manager.streamInputs.listenToMicIncludesOutputEffects
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.streamInputs.listenToMicIncludesOutputEffects)
                            DB.Manager.streamInputs.listenToMicIncludesOutputEffects = isChecked;
                    }
                }

                EeSwitch {
                    id: inactivityTimerEnable

                    label: i18n("Enable the inactivity timeout") // qmllint disable
                    subtitle: i18n("When all streams are inactive, Easy Effects pipeline remains loaded for an extra amount of time.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.inactivityTimerEnable
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.inactivityTimerEnable)
                            DB.Manager.main.inactivityTimerEnable = isChecked;
                    }
                }

                EeSpinBox {
                    id: inactivityTimeout

                    label: i18n("Inactivity timeout") // qmllint disable
                    from: DB.Manager.main.getMinValue("inactivityTimeout")
                    to: DB.Manager.main.getMaxValue("inactivityTimeout")
                    value: DB.Manager.main.inactivityTimeout
                    decimals: 0
                    stepSize: 1
                    unit: i18n("s")
                    enabled: DB.Manager.main.inactivityTimerEnable
                    onValueModified: v => {
                        DB.Manager.main.inactivityTimeout = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Level meters label") // qmllint disable
                    subtitle: i18n("The time it takes for the level meter labels to be updated when the current level is below the last highest one.") // qmllint disable
                    maximumLineCount: -1
                    from: DB.Manager.main.getMinValue("levelMetersLabelTimer")
                    to: DB.Manager.main.getMaxValue("levelMetersLabelTimer")
                    value: DB.Manager.main.levelMetersLabelTimer
                    decimals: 0
                    stepSize: 1
                    unit: i18n("ms")
                    enabled: DB.Manager.main.levelMetersLabelTimer
                    onValueModified: v => {
                        DB.Manager.main.levelMetersLabelTimer = v;
                    }
                }
            }
        }
    }

    readonly property Component spectrumPage: Component {
        FormCard.FormCardPage {
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

                    label: i18n("Audio delay compensation") // qmllint disable
                    from: 0
                    to: 1000
                    value: DB.Manager.spectrum.avsyncDelay
                    decimals: 0
                    stepSize: 1
                    unit: i18n("ms")
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

                    text: i18n("Color scheme") // qmllint disable
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

                    text: i18n("Color theme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DB.Manager.spectrum.spectrumColorTheme
                    editable: false
                    model: [i18n("Green"), i18n("Green neon"), i18n("Mix"), i18n("Orange"), i18n("Yellow"), i18n("Blue"), i18n("Purple"), i18n("Grey")]// qmllint disable
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

                    label: i18n("Dynamic scale") // qmllint disable
                    isChecked: DB.Manager.spectrum.dynamicYScale
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.spectrum.dynamicYScale)
                            DB.Manager.spectrum.dynamicYScale = isChecked;
                    }
                }

                EeSwitch {
                    id: logarithimicHorizontalAxis

                    label: i18n("Logarithmic frequency axis") // qmllint disable
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
                    unit: i18n("px")
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
                    unit: i18n("Hz")
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
                    unit: i18n("Hz")
                    onValueModified: v => {
                        DB.Manager.spectrum.maximumFrequency = v;
                    }
                }
            }
        }
    }

    readonly property Component databasePage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSpinBox {
                    label: i18n("Database autosave interval") // qmllint disable
                    from: DB.Manager.main.getMinValue("databaseAutosaveInterval")
                    to: DB.Manager.main.getMaxValue("databaseAutosaveInterval")
                    value: DB.Manager.main.databaseAutosaveInterval
                    decimals: 0
                    stepSize: 1
                    unit: i18n("ms")
                    onValueModified: v => {
                        DB.Manager.main.databaseAutosaveInterval = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Most used presets") // qmllint disable
                    from: DB.Manager.main.getMinValue("maxMostUsedPresets")
                    to: DB.Manager.main.getMaxValue("maxMostUsedPresets")
                    value: DB.Manager.main.maxMostUsedPresets
                    decimals: 0
                    stepSize: 1
                    onValueModified: v => {
                        DB.Manager.main.maxMostUsedPresets = v;
                    }
                }
            }
        }
    }

    readonly property Component stylePage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: reducePluginsListControls

                    label: i18n("Reduce effects list controls") // qmllint disable
                    subtitle: i18n("Action buttons in the effects list are moved inside a context menu for a more compact view.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.reducePluginsListControls
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.reducePluginsListControls) {
                            DB.Manager.main.reducePluginsListControls = isChecked;
                        }
                    }
                }
            }
        }
    }

    readonly property Component experimentalPage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: xdgGlobalShortcuts

                    label: i18n("Global shortcuts") // qmllint disable
                    subtitle: i18n("Enables support for XDG global shortcuts.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.xdgGlobalShortcuts
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.xdgGlobalShortcuts)
                            DB.Manager.main.xdgGlobalShortcuts = isChecked;
                    }
                }

                EeSwitch {
                    id: showNativePluginUi

                    label: i18n("Native window of effects") // qmllint disable
                    subtitle: i18n("Allows the native user interface of effects to be shown/hidden.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DB.Manager.main.showNativePluginUi
                    onCheckedChanged: {
                        if (isChecked !== DB.Manager.main.showNativePluginUi)
                            DB.Manager.main.showNativePluginUi = isChecked;
                    }
                }

                EeSpinBox {
                    id: lv2uiUpdateFrequency

                    label: i18n("Update frequency") // qmllint disable
                    subtitle: i18n("Related to LV2 plugins.") // qmllint disable
                    maximumLineCount: -1
                    from: 1
                    to: 60
                    value: DB.Manager.main.lv2uiUpdateFrequency
                    decimals: 0
                    stepSize: 1
                    unit: i18n("Hz")
                    enabled: DB.Manager.main.showNativePluginUi
                    onValueModified: v => {
                        DB.Manager.main.lv2uiUpdateFrequency = v;
                    }
                }
            }

            Item {
                implicitHeight: Kirigami.Units.gridUnit
            }
        }
    }
}
