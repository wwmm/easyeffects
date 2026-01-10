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

import QtGraphs
import QtQml // Despite of what Qt extension says this import is needed. We crash without it
import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.settings as KirigamiSettings

KirigamiSettings.ConfigurationView {
    id: preferencesSheet

    modules: [
        KirigamiSettings.ConfigurationModule {
            moduleId: "appearance"
            icon.name: "backgroundtool-symbolic"
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
            moduleId: "style"
            icon.name: "preferences-desktop-theme-global-symbolic"
            text: i18n("Style") // qmllint disable
            page: () => stylePage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "presets"
            icon.name: "user-bookmarks-symbolic"
            text: i18n("Presets") // qmllint disable
            page: () => presetsPage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "database"
            icon.name: "server-database-symbolic"
            text: i18n("Database") // qmllint disable
            page: () => databasePage
        },
        KirigamiSettings.ConfigurationModule {
            moduleId: "experimental"
            icon.name: "flag-symbolic"
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
                    isChecked: DbMain.enableServiceMode
                    onCheckedChanged: {
                        if (isChecked !== DbMain.enableServiceMode)
                            DbMain.enableServiceMode = isChecked;
                    }
                }

                EeSwitch {
                    id: autostartOnLogin

                    label: i18n("Autostart on login") // qmllint disable
                    subtitle: i18n("Easy Effects is launched at user session startup.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.autostartOnLogin
                    onCheckedChanged: {
                        if (isChecked !== DbMain.autostartOnLogin) {
                            DbMain.autostartOnLogin = isChecked;
                        }
                    }
                }

                EeSwitch {
                    id: showTrayIcon

                    label: i18n("Show the tray icon") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.showTrayIcon && canUseSysTray // qmllint disable
                    enabled: canUseSysTray // qmllint disable
                    onCheckedChanged: {
                        if (isChecked !== DbMain.showTrayIcon)
                            DbMain.showTrayIcon = isChecked;
                    }
                }

                EeSwitch {
                    id: noWindowAfterStarting

                    label: i18n("Do not show the application window") // qmllint disable
                    subtitle: i18n("Do not show the window when the application starts. In general this option should not be enabled. It is intended only as a workaround for systems that do not work well with the autostart of Easy Effects.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.noWindowAfterStarting // qmllint disable
                    onCheckedChanged: {
                        if (isChecked !== DbMain.noWindowAfterStarting)
                            DbMain.noWindowAfterStarting = isChecked;
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
                    maximumLineCount: -1
                    isChecked: DbMain.processAllOutputs
                    onCheckedChanged: {
                        if (isChecked !== DbMain.processAllOutputs)
                            DbMain.processAllOutputs = isChecked;
                    }
                }

                EeSwitch {
                    id: processAllInputs

                    label: i18n("Process all input streams") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.processAllInputs
                    onCheckedChanged: {
                        if (isChecked !== DbMain.processAllInputs)
                            DbMain.processAllInputs = isChecked;
                    }
                }

                EeSwitch {
                    id: excludeMonitorStreams

                    label: i18n("Ignore streams from monitor of devices") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.excludeMonitorStreams
                    onCheckedChanged: {
                        if (isChecked !== DbMain.excludeMonitorStreams)
                            DbMain.excludeMonitorStreams = isChecked;
                    }
                }

                EeSwitch {
                    id: useCubicVolumes

                    label: i18n("Use cubic volume") // qmllint disable
                    subtitle: i18n("Use cubic scale for stream volume rather than the linear one. Low percentages results in a lower perceived volume while the loudness appears to change more evenly across the slider.")
                    maximumLineCount: -1
                    isChecked: DbMain.useCubicVolumes
                    onCheckedChanged: {
                        if (isChecked !== DbMain.useCubicVolumes)
                            DbMain.useCubicVolumes = isChecked;
                    }
                }

                EeSwitch {
                    id: ignoreSystemNotifications

                    label: i18n("Ignore system notifications") // qmllint disable
                    subtitle: i18n("Processing system notifications may cause crackling sound, so it would be better to ignore them, but for some presets it may be recommended since they could sound too loud.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.ignoreSystemNotifications
                    onCheckedChanged: {
                        if (isChecked !== DbMain.ignoreSystemNotifications)
                            DbMain.ignoreSystemNotifications = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMic

                    label: i18n("Enable/disable input monitoring") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbStreamInputs.listenToMic
                    onCheckedChanged: {
                        if (isChecked !== DbStreamInputs.listenToMic)
                            DbStreamInputs.listenToMic = isChecked;
                    }
                }

                EeSwitch {
                    id: listenToMicIncludesOutputEffects

                    label: i18n("Mic monitor plays to output effects pipeline") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbStreamInputs.listenToMicIncludesOutputEffects
                    onCheckedChanged: {
                        if (isChecked !== DbStreamInputs.listenToMicIncludesOutputEffects)
                            DbStreamInputs.listenToMicIncludesOutputEffects = isChecked;
                    }
                }

                EeSwitch {
                    id: processAllcopyFilterInputBuffersOutputs

                    label: i18n("Copy the input audio buffer") // qmllint disable
                    subtitle: i18n("Use a copy of the input audio buffer given by the sound server when processing audio inside Easy Effects plugins. This fixes audio glitches that can happen when external applications are recording from Easy Effects virtual devices monitors.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.copyFilterInputBuffers
                    onCheckedChanged: {
                        if (isChecked !== DbMain.copyFilterInputBuffers)
                            DbMain.copyFilterInputBuffers = isChecked;
                    }
                }

                EeSwitch {
                    id: inactivityTimerEnable

                    label: i18n("Enable the inactivity timeout") // qmllint disable
                    subtitle: i18n("When all streams are inactive, Easy Effects pipeline remains loaded for an extra amount of time.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.inactivityTimerEnable
                    onCheckedChanged: {
                        if (isChecked !== DbMain.inactivityTimerEnable)
                            DbMain.inactivityTimerEnable = isChecked;
                    }
                }

                EeSpinBox {
                    id: inactivityTimeout

                    label: i18n("Inactivity timeout") // qmllint disable
                    maximumLineCount: -1
                    from: DbMain.getMinValue("inactivityTimeout")
                    to: DbMain.getMaxValue("inactivityTimeout")
                    value: DbMain.inactivityTimeout
                    decimals: 0
                    stepSize: 1
                    unit: Units.s
                    enabled: DbMain.inactivityTimerEnable
                    onValueModified: v => {
                        DbMain.inactivityTimeout = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Level meters frame rate cap") // qmllint disable
                    subtitle: i18n("Maximum level meter update rate.") // qmllint disable
                    maximumLineCount: -1
                    from: DbMain.getMinValue("levelMetersFpsCap")
                    to: DbMain.getMaxValue("levelMetersFpsCap")
                    value: DbMain.levelMetersFpsCap
                    decimals: 0
                    stepSize: 1
                    unit: Units.fps
                    onValueModified: v => {
                        DbMain.levelMetersFpsCap = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Level meters label") // qmllint disable
                    subtitle: i18n("The time it takes for the level meter labels to be updated when the current level is below the last highest one.") // qmllint disable
                    maximumLineCount: -1
                    from: DbMain.getMinValue("levelMetersLabelTimer")
                    to: DbMain.getMaxValue("levelMetersLabelTimer")
                    value: DbMain.levelMetersLabelTimer
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    enabled: DbMain.levelMetersLabelTimer
                    onValueModified: v => {
                        DbMain.levelMetersLabelTimer = v;
                    }
                }

                EeSwitch {
                    id: enableLevelMetersAnimation

                    label: i18n("Enable level meters animation") // qmllint disable
                    isChecked: DbMain.enableLevelMetersAnimation
                    onCheckedChanged: {
                        if (isChecked !== DbMain.enableLevelMetersAnimation)
                            DbMain.enableLevelMetersAnimation = isChecked;
                    }
                }

                EeSpinBox {
                    label: i18n("Level meters animation duration") // qmllint disable
                    from: DbMain.getMinValue("levelMetersAnimationDuration")
                    to: DbMain.getMaxValue("levelMetersAnimationDuration")
                    value: DbMain.levelMetersAnimationDuration
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    enabled: DbMain.enableLevelMetersAnimation
                    onValueModified: v => {
                        DbMain.levelMetersAnimationDuration = v;
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
                    maximumLineCount: -1
                    isChecked: DbSpectrum.state
                    onCheckedChanged: {
                        if (isChecked !== DbSpectrum.state)
                            DbSpectrum.state = isChecked;
                    }
                }

                EeSpinBox {
                    id: avsyncDelay

                    label: i18n("Audio delay compensation") // qmllint disable
                    maximumLineCount: -1
                    from: 0
                    to: 1000
                    value: DbSpectrum.avsyncDelay
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    onValueModified: v => {
                        DbSpectrum.avsyncDelay = v;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Graph") // qmllint disable
            }

            FormCard.FormCard {
                FormCard.FormComboBoxDelegate {
                    id: spectrumShape

                    text: i18n("Shape") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DbSpectrum.spectrumShape
                    editable: false
                    model: [i18n("Bars"), i18n("Lines"), i18n("Dots"), i18n("Area")] // qmllint disable
                    onActivated: idx => {
                        if (idx !== DbSpectrum.spectrumShape)
                            DbSpectrum.spectrumShape = idx;
                    }
                }

                EeSwitch {
                    id: dynamicYScale

                    label: i18n("Dynamic scale") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbSpectrum.dynamicYScale
                    onCheckedChanged: {
                        if (isChecked !== DbSpectrum.dynamicYScale)
                            DbSpectrum.dynamicYScale = isChecked;
                    }
                }

                EeSwitch {
                    id: logarithimicHorizontalAxis

                    label: i18n("Logarithmic frequency axis") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbSpectrum.logarithimicHorizontalAxis
                    onCheckedChanged: {
                        if (isChecked !== DbSpectrum.logarithimicHorizontalAxis)
                            DbSpectrum.logarithimicHorizontalAxis = isChecked;
                    }
                }

                EeSpinBox {
                    id: nPoints

                    label: i18n("Points") // qmllint disable
                    maximumLineCount: -1
                    from: 2
                    to: 2048
                    value: DbSpectrum.nPoints
                    decimals: 0
                    stepSize: 1
                    onValueModified: v => {
                        DbSpectrum.nPoints = v;
                    }
                }

                EeSpinBox {
                    id: height

                    label: i18n("Height") // qmllint disable
                    maximumLineCount: -1
                    from: 100
                    to: 1000
                    value: DbSpectrum.height
                    decimals: 0
                    stepSize: 1
                    unit: i18n("px")
                    onValueModified: v => {
                        DbSpectrum.height = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Spectrum frame rate cap") // qmllint disable
                    subtitle: i18n("Maximum spectrum update rate.") // qmllint disable
                    maximumLineCount: -1
                    from: DbSpectrum.getMinValue("spectrumFpsCap")
                    to: DbSpectrum.getMaxValue("spectrumFpsCap")
                    value: DbSpectrum.spectrumFpsCap
                    decimals: 0
                    stepSize: 1
                    unit: Units.fps
                    onValueModified: v => {
                        DbSpectrum.spectrumFpsCap = v;
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
                    maximumLineCount: -1
                    from: DbSpectrum.getMinValue("minimumFrequency")
                    to: DbSpectrum.getMaxValue("minimumFrequency")
                    value: DbSpectrum.minimumFrequency
                    decimals: 0
                    stepSize: 10
                    unit: Units.hz
                    onValueModified: v => {
                        DbSpectrum.minimumFrequency = v;
                    }
                }

                EeSpinBox {
                    id: maximumFrequency

                    label: i18n("Maximum") // qmllint disable
                    maximumLineCount: -1
                    from: DbSpectrum.getMinValue("maximumFrequency")
                    to: DbSpectrum.getMaxValue("maximumFrequency")
                    value: DbSpectrum.maximumFrequency
                    decimals: 0
                    stepSize: 10
                    unit: Units.hz
                    onValueModified: v => {
                        DbSpectrum.maximumFrequency = v;
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
                    maximumLineCount: -1
                    from: DbMain.getMinValue("databaseAutosaveInterval")
                    to: DbMain.getMaxValue("databaseAutosaveInterval")
                    value: DbMain.databaseAutosaveInterval
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    onValueModified: v => {
                        DbMain.databaseAutosaveInterval = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Most used presets") // qmllint disable
                    maximumLineCount: -1
                    from: DbMain.getMinValue("maxMostUsedPresets")
                    to: DbMain.getMaxValue("maxMostUsedPresets")
                    value: DbMain.maxMostUsedPresets
                    decimals: 0
                    stepSize: 1
                    onValueModified: v => {
                        DbMain.maxMostUsedPresets = v;
                    }
                }
            }
        }
    }

    readonly property Component stylePage: Component {
        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("Window") // qmllint disable
            }

            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSwitch {
                    id: forceBreezeTheme

                    label: i18n("Force KDE's Breeze theme when Easy Effects starts") // qmllint disable
                    subtitle: i18n("It is necessary to restart the service for changes in this setting to take effect.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.forceBreezeTheme
                    onCheckedChanged: {
                        if (isChecked !== DbMain.forceBreezeTheme) {
                            DbMain.forceBreezeTheme = isChecked;
                        }
                    }
                }

                EeSwitch {
                    id: reducePluginsListControls

                    label: i18n("Reduce effects list controls") // qmllint disable
                    subtitle: i18n("Action buttons in the effects list are moved inside a context menu for a more compact view.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.reducePluginsListControls
                    onCheckedChanged: {
                        if (isChecked !== DbMain.reducePluginsListControls) {
                            DbMain.reducePluginsListControls = isChecked;
                        }
                    }
                }

                FormCard.FormComboBoxDelegate {
                    text: i18n("Color scheme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: KColorManager.activeScheme()
                    editable: false
                    model: KColorManager.model
                    textRole: "display"
                    onActivated: idx => {
                        KColorManager.activateScheme(idx);
                    }
                }

                EeSwitch {
                    id: englishLanguage

                    label: i18n("Force English language") // qmllint disable
                    subtitle: i18n("Do not translate the user interface and show text in English language. Useful if the translation in the local language is incomplete or incomprehensible. A restart of the service is required to apply this option.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.englishLanguage // qmllint disable
                    onCheckedChanged: {
                        if (isChecked !== DbMain.englishLanguage)
                            DbMain.englishLanguage = isChecked;
                    }
                }
            }

            FormCard.FormHeader {
                title: i18n("Graphs") // qmllint disable
            }

            FormCard.FormCard {
                id: graphFormCard

                readonly property bool useUserTheme: DbGraph.colorTheme === GraphsTheme.Theme.UserDefined ? true : false

                FormCard.FormComboBoxDelegate {
                    id: chartColorScheme

                    text: i18n("Color scheme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DbGraph.colorScheme
                    editable: false
                    model: [i18n("Automatic"), i18n("Light"), i18n("Dark")] // qmllint disable
                    enabled: !graphFormCard.useUserTheme
                    onActivated: idx => {
                        if (idx !== DbGraph.colorScheme)
                            DbGraph.colorScheme = idx;
                    }
                }

                FormCard.FormComboBoxDelegate {
                    id: chartColorTheme

                    text: i18n("Color theme") // qmllint disable
                    displayMode: FormCard.FormComboBoxDelegate.ComboBox
                    currentIndex: DbGraph.colorTheme
                    editable: false
                    model: [i18n("Green"), i18n("Green neon"), i18n("Mix"), i18n("Orange"), i18n("Yellow"), i18n("Blue"), i18n("Purple"), i18n("Grey"), i18n("User")] // qmllint disable
                    onActivated: idx => {
                        if (idx !== DbGraph.colorTheme)
                            DbGraph.colorTheme = idx;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Background")
                    color: DbGraph.backgroundColor
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.backgroundColor = color;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Plot area background")
                    color: DbGraph.plotAreaBackgroundColor
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.plotAreaBackgroundColor = color;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Series colors")
                    color: DbGraph.seriesColors
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.seriesColors = color;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Axis labels text color")
                    color: DbGraph.labelTextColor
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.labelTextColor = color;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Axis labels background color")
                    color: DbGraph.labelBackgroundColor
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.labelBackgroundColor = color;
                    }
                }

                FormCard.FormColorDelegate {
                    text: i18nc("@label", "Border color")
                    color: DbGraph.borderColors
                    enabled: graphFormCard.useUserTheme
                    onColorChanged: {
                        DbGraph.borderColors = color;
                    }
                }

                EeSpinBox {
                    label: i18n("Line width") // qmllint disable
                    from: DbGraph.getMinValue("lineWidth")
                    to: DbGraph.getMaxValue("lineWidth")
                    value: DbGraph.lineWidth
                    decimals: 2
                    stepSize: 0.01
                    onValueModified: v => {
                        DbGraph.lineWidth = v;
                    }
                }

                EeSwitch {
                    id: gridVisible

                    label: i18n("Show grid lines") // qmllint disable
                    isChecked: DbGraph.gridVisible
                    onCheckedChanged: {
                        if (isChecked !== DbGraph.gridVisible) {
                            DbGraph.gridVisible = isChecked;
                        }
                    }
                }
            }
        }
    }

    readonly property Component presetsPage: Component {
        FormCard.FormCardPage {
            FormCard.FormCard {
                Layout.topMargin: Kirigami.Units.gridUnit

                EeSpinBox {
                    label: i18n("Autoload delay") // qmllint disable
                    subtitle: i18n("Delay between receiving a new device route and the call to autoload a preset.") // qmllint disable
                    maximumLineCount: -1
                    from: DbMain.getMinValue("presetsAutoloadInterval")
                    to: DbMain.getMaxValue("presetsAutoloadInterval")
                    value: DbMain.presetsAutoloadInterval
                    decimals: 0
                    stepSize: 1
                    unit: Units.ms
                    onValueModified: v => {
                        DbMain.presetsAutoloadInterval = v;
                    }
                }

                EeSwitch {
                    id: ignorePresetBlocklist

                    label: i18n("Ignore the blocklist from presets") // qmllint disable
                    subtitle: i18n("If enabled, the list of excluded apps set in the loaded preset is ignored and does not replace the one set by the user.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.ignorePresetBlocklist
                    onCheckedChanged: {
                        if (isChecked !== DbMain.ignorePresetBlocklist) {
                            DbMain.ignorePresetBlocklist = isChecked;
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
                    isChecked: DbMain.xdgGlobalShortcuts
                    onCheckedChanged: {
                        if (isChecked !== DbMain.xdgGlobalShortcuts)
                            DbMain.xdgGlobalShortcuts = isChecked;
                    }
                }

                EeSwitch {
                    id: showNativePluginUi

                    label: i18n("Native window of effects") // qmllint disable
                    subtitle: i18n("Allows the native user interface of effects to be shown/hidden.") // qmllint disable
                    maximumLineCount: -1
                    isChecked: DbMain.showNativePluginUi
                    onCheckedChanged: {
                        if (isChecked !== DbMain.showNativePluginUi)
                            DbMain.showNativePluginUi = isChecked;
                    }
                }

                EeSpinBox {
                    id: lv2uiUpdateFrequency

                    label: i18n("Update frequency") // qmllint disable
                    subtitle: i18n("Related to LV2 plugins.") // qmllint disable
                    maximumLineCount: -1
                    from: 1
                    to: 60
                    value: DbMain.lv2uiUpdateFrequency
                    decimals: 0
                    stepSize: 1
                    unit: Units.hz
                    enabled: DbMain.showNativePluginUi
                    onValueModified: v => {
                        DbMain.lv2uiUpdateFrequency = v;
                    }
                }
            }

            Item {
                implicitHeight: Kirigami.Units.gridUnit
            }
        }
    }
}
