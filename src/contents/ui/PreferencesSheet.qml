import EEdbm
import QtQml
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.OverlaySheet {
    id: preferencesSheet

    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: appWindow.height * 0.7
    title: i18n("Preferences")
    parent: applicationWindow().overlay
    onVisibleChanged: {
        if (!preferencesSheet.visible) {
            while (stack.depth > 1)stack.pop()
            headerTitle.text = i18n("Preferences");
        }
    }

    Component {
        id: servicePage

        Kirigami.Page {
            ColumnLayout {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                FormCard.FormCard {
                    EeSwitch {
                        id: enableServiceMode

                        label: i18n("Enable Service Mode")
                        isChecked: EEdbm.main.enableServiceMode
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.enableServiceMode)
                                EEdbm.main.enableServiceMode = isChecked;

                        }
                    }

                    EeSwitch {
                        id: autostartOnLogin

                        label: i18n("Autostart on Login")
                        // isChecked: EEdbm.main.autostartOnLogin
                        onCheckedChanged: {
                        }
                    }

                    EeSwitch {
                        id: showTrayIcon

                        label: i18n("Show the Tray Icon")
                        isChecked: EEdbm.main.showTrayIcon && canUseSysTray
                        enabled: canUseSysTray
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.showTrayIcon)
                                EEdbm.main.showTrayIcon = isChecked;

                        }
                    }

                }

            }

        }

    }

    Component {
        id: audioPage

        Kirigami.Page {
            ColumnLayout {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                FormCard.FormCard {
                    EeSwitch {
                        id: processAllOutputs

                        label: i18n("Process All Output Streams")
                        isChecked: EEdbm.main.processAllOutputs
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.processAllOutputs)
                                EEdbm.main.processAllOutputs = isChecked;

                        }
                    }

                    EeSwitch {
                        id: processAllInputs

                        label: i18n("Process All Input Streams")
                        isChecked: EEdbm.main.processAllInputs
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.processAllInputs)
                                EEdbm.main.processAllInputs = isChecked;

                        }
                    }

                    EeSwitch {
                        id: excludeMonitorStreams

                        label: i18n("Ignore Streams from Monitor of Devices")
                        isChecked: EEdbm.main.excludeMonitorStreams
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.excludeMonitorStreams)
                                EEdbm.main.excludeMonitorStreams = isChecked;

                        }
                    }

                    EeSwitch {
                        id: useCubicVolumes

                        label: i18n("Use Cubic Volume")
                        isChecked: EEdbm.main.useCubicVolumes
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.useCubicVolumes)
                                EEdbm.main.useCubicVolumes = isChecked;

                        }
                    }

                    EeSwitch {
                        id: inactivityTimerEnable

                        label: i18n("Enable the Inactivity Timeout")
                        isChecked: EEdbm.main.inactivityTimerEnable
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.inactivityTimerEnable)
                                EEdbm.main.inactivityTimerEnable = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: inactivityTimeout

                        label: i18n("Inactivity Timeout")
                        subtitle: i18n("Input Pipeline")
                        from: 1
                        to: 3600
                        value: EEdbm.main.inactivityTimeout
                        decimals: 0
                        stepSize: 1
                        unit: "s"
                        enabled: EEdbm.main.inactivityTimerEnable
                        onValueModified: (v) => {
                            EEdbm.main.inactivityTimeout = v;
                        }
                    }

                    EeSpinBox {
                        id: metersUpdateInterval

                        label: i18n("Update Interval")
                        subtitle: i18n("Related to Level Meters and Spectrum")
                        from: 10
                        to: 1000
                        value: EEdbm.main.metersUpdateInterval
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            EEdbm.main.metersUpdateInterval = v;
                        }
                    }

                }

            }

        }

    }

    Component {
        id: spectrumPage

        Kirigami.Page {
            ColumnLayout {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                FormCard.FormHeader {
                    title: i18n("State")
                }

                FormCard.FormCard {
                    EeSwitch {
                        id: spectrumState

                        label: i18n("Enabled")
                        isChecked: EEdbm.spectrum.state
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.spectrum.state)
                                EEdbm.spectrum.state = isChecked;

                        }
                    }

                }

                FormCard.FormHeader {
                    title: i18n("Style")
                }

                FormCard.FormCard {
                    FormCard.FormComboBoxDelegate {
                        id: spectrumShape

                        text: i18n("Shape")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: EEdbm.spectrum.spectrumShape
                        editable: false
                        model: [i18n("Bars"), i18n("Lines"), i18n("Dots"), i18n("Area")]
                        onActivated: (idx) => {
                            if (idx !== EEdbm.spectrum.spectrumShape)
                                EEdbm.spectrum.spectrumShape = idx;

                        }
                    }

                    EeSwitch {
                        label: i18n("Enable OpenGL Acceleration")
                        isChecked: EEdbm.spectrum.useOpenGL
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.spectrum.useOpenGL)
                                EEdbm.spectrum.useOpenGL = isChecked;

                        }
                    }

                    EeSwitch {
                        id: dynamicYScale

                        label: i18n("Dynamic Scale")
                        isChecked: EEdbm.spectrum.dynamicYScale
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.spectrum.dynamicYScale)
                                EEdbm.spectrum.dynamicYScale = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: nPoints

                        label: i18n("Points")
                        from: 2
                        to: 2048
                        value: EEdbm.spectrum.nPoints
                        decimals: 0
                        stepSize: 1
                        onValueModified: (v) => {
                            EEdbm.spectrum.nPoints = v;
                        }
                    }

                    EeSpinBox {
                        id: height

                        label: i18n("Height")
                        from: 100
                        to: 1000
                        value: EEdbm.spectrum.height
                        decimals: 0
                        stepSize: 1
                        unit: "px"
                        onValueModified: (v) => {
                            EEdbm.spectrum.height = v;
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
                        value: EEdbm.spectrum.minimumFrequency
                        decimals: 0
                        stepSize: 10
                        unit: "Hz"
                        onValueModified: (v) => {
                            EEdbm.spectrum.minimumFrequency = v;
                        }
                    }

                    EeSpinBox {
                        id: maximumFrequency

                        label: i18n("Maximum")
                        from: 120
                        to: 22000
                        value: EEdbm.spectrum.maximumFrequency
                        decimals: 0
                        stepSize: 10
                        unit: "Hz"
                        onValueModified: (v) => {
                            EEdbm.spectrum.maximumFrequency = v;
                        }
                    }

                }

            }

        }

    }

    Component {
        id: experimentalPage

        Kirigami.Page {
            ColumnLayout {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                FormCard.FormCard {
                    EeSwitch {
                        id: showNativePluginUi

                        label: i18n("Native Plugin Window")
                        subtitle: i18n("Allows The Native Plugin Window to be Shown/Hidden")
                        isChecked: EEdbm.main.showNativePluginUi
                        onCheckedChanged: {
                            if (isChecked !== EEdbm.main.showNativePluginUi)
                                EEdbm.main.showNativePluginUi = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: lv2uiUpdateFrequency

                        label: i18n("Update Frequency")
                        subtitle: i18n("Related to LV2 Plugins")
                        from: 1
                        to: 60
                        value: EEdbm.main.lv2uiUpdateFrequency
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        enabled: EEdbm.main.showNativePluginUi
                        onValueModified: (v) => {
                            EEdbm.main.lv2uiUpdateFrequency = v;
                        }
                    }

                }

            }

        }

    }

    Controls.StackView {
        id: stack

        anchors.fill: parent
        implicitWidth: Kirigami.Units.gridUnit * 30
        implicitHeight: stack.currentItem.implicitHeight

        initialItem: Kirigami.Page {
            id: childItem

            ColumnLayout {
                anchors.fill: parent

                FormCard.FormCard {
                    FormCard.FormButtonDelegate {
                        id: serviceButton

                        icon.name: "services-symbolic"
                        text: i18n("Background Service")
                        onClicked: {
                            while (stack.depth > 1)stack.pop()
                            stack.push(servicePage);
                            headerTitle.text = text;
                        }
                    }

                    FormCard.FormButtonDelegate {
                        id: audioButton

                        icon.name: "folder-sound-symbolic"
                        text: i18n("Audio")
                        onClicked: {
                            while (stack.depth > 1)stack.pop()
                            stack.push(audioPage);
                            headerTitle.text = text;
                        }
                    }

                    FormCard.FormButtonDelegate {
                        id: spectrumButton

                        icon.name: "folder-chart-symbolic"
                        text: i18n("Spectrum Analyzer")
                        onClicked: {
                            while (stack.depth > 1)stack.pop()
                            stack.push(spectrumPage);
                            headerTitle.text = text;
                        }
                    }

                    FormCard.FormButtonDelegate {
                        id: experimentalButton

                        icon.name: "emblem-warning"
                        text: i18n("Experimental Features")
                        onClicked: {
                            while (stack.depth > 1)stack.pop()
                            stack.push(experimentalPage);
                            headerTitle.text = text;
                        }
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
                while (stack.depth > 1)stack.pop()
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
