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
                        id: launchServiceOnLogin

                        label: i18n("Launch Service at System Startup")
                        // isChecked: EEdb.addRandom
                        onCheckedChanged: {
                        }
                    }

                    EeSwitch {
                        id: showTrayIcon

                        label: i18n("Show the Tray Icon")
                        isChecked: EEdb.showTrayIcon
                        onCheckedChanged: {
                            if (isChecked !== EEdb.showTrayIcon)
                                EEdb.showTrayIcon = isChecked;

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
                        isChecked: EEdb.processAllOutputs
                        onCheckedChanged: {
                            if (isChecked !== EEdb.processAllOutputs)
                                EEdb.processAllOutputs = isChecked;

                        }
                    }

                    EeSwitch {
                        id: processAllInputs

                        label: i18n("Process All Input Streams")
                        isChecked: EEdb.processAllInputs
                        onCheckedChanged: {
                            if (isChecked !== EEdb.processAllInputs)
                                EEdb.processAllInputs = isChecked;

                        }
                    }

                    EeSwitch {
                        id: excludeMonitorStreams

                        label: i18n("Ignore Streams from Monitor of Devices")
                        isChecked: EEdb.excludeMonitorStreams
                        onCheckedChanged: {
                            if (isChecked !== EEdb.excludeMonitorStreams)
                                EEdb.excludeMonitorStreams = isChecked;

                        }
                    }

                    EeSwitch {
                        id: useCubicVolumes

                        label: i18n("Use Cubic Volume")
                        isChecked: EEdb.useCubicVolumes
                        onCheckedChanged: {
                            if (isChecked !== EEdb.useCubicVolumes)
                                EEdb.useCubicVolumes = isChecked;

                        }
                    }

                    EeSwitch {
                        id: inactivityTimerEnable

                        label: i18n("Enable the Inactivity Timeout")
                        isChecked: EEdb.inactivityTimerEnable
                        onCheckedChanged: {
                            if (isChecked !== EEdb.inactivityTimerEnable)
                                EEdb.inactivityTimerEnable = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: inactivityTimeout

                        label: i18n("Inactivity Timeout")
                        subtitle: i18n("Input Pipeline")
                        from: 1
                        to: 3600
                        value: EEdb.inactivityTimeout
                        decimals: 0
                        stepSize: 1
                        unit: "s"
                        enabled: EEdb.inactivityTimerEnable
                        onValueModified: (v) => {
                            EEdb.inactivityTimeout = v;
                        }
                    }

                    EeSpinBox {
                        id: metersUpdateInterval

                        label: i18n("Update Interval")
                        subtitle: i18n("Related to Level Meters and Spectrum")
                        from: 10
                        to: 1000
                        value: EEdb.metersUpdateInterval
                        decimals: 0
                        stepSize: 1
                        unit: "ms"
                        onValueModified: (v) => {
                            EEdb.metersUpdateInterval = v;
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
                        isChecked: EEdbSpectrum.state
                        onCheckedChanged: {
                            if (isChecked !== EEdbSpectrum.state)
                                EEdbSpectrum.state = isChecked;

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
                        currentIndex: EEdbSpectrum.spectrumShape
                        editable: false
                        model: [i18n("Bars"), i18n("Lines"), i18n("Dots"), i18n("Area")]
                        onActivated: (idx) => {
                            if (idx !== EEdbSpectrum.spectrumShape)
                                EEdbSpectrum.spectrumShape = idx;

                        }
                    }

                    FormCard.FormComboBoxDelegate {
                        id: theme

                        text: i18n("Theme")
                        displayMode: FormCard.FormComboBoxDelegate.ComboBox
                        currentIndex: EEdbSpectrum.theme
                        editable: false
                        model: [i18n("Light"), i18n("Cerulean Blue"), i18n("Dark"), i18n("Sand Brown"), i18n("Ncs Blue"), i18n("High Contras"), i18n("Icy Blue"), i18n("Qt")]
                        onActivated: (idx) => {
                            if (idx !== EEdbSpectrum.theme)
                                EEdbSpectrum.theme = idx;

                        }
                    }

                    EeSwitch {
                        label: i18n("Enable OpenGL Acceleration")
                        isChecked: EEdbSpectrum.useOpenGL
                        onCheckedChanged: {
                            if (isChecked !== EEdbSpectrum.useOpenGL)
                                EEdbSpectrum.useOpenGL = isChecked;

                        }
                    }

                    EeSwitch {
                        id: dynamicYScale

                        label: i18n("Dynamic Scale")
                        isChecked: EEdbSpectrum.dynamicYScale
                        onCheckedChanged: {
                            if (isChecked !== EEdbSpectrum.dynamicYScale)
                                EEdbSpectrum.dynamicYScale = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: nPoints

                        label: i18n("Points")
                        from: 2
                        to: 2048
                        value: EEdbSpectrum.nPoints
                        decimals: 0
                        stepSize: 1
                        onValueModified: (v) => {
                            EEdbSpectrum.nPoints = v;
                        }
                    }

                    EeSpinBox {
                        id: height

                        label: i18n("Height")
                        from: 100
                        to: 1000
                        value: EEdbSpectrum.height
                        decimals: 0
                        stepSize: 1
                        unit: "px"
                        onValueModified: (v) => {
                            EEdbSpectrum.height = v;
                        }
                    }

                    EeSpinBox {
                        id: lineWidth

                        label: i18n("Line Width")
                        from: 10
                        to: 1000
                        value: EEdbSpectrum.lineWidth
                        decimals: 1
                        stepSize: 0.1
                        unit: "px"
                        onValueModified: (v) => {
                            EEdbSpectrum.lineWidth = v;
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
                        value: EEdbSpectrum.minimumFrequency
                        decimals: 0
                        stepSize: 10
                        unit: "Hz"
                        onValueModified: (v) => {
                            EEdbSpectrum.minimumFrequency = v;
                        }
                    }

                    EeSpinBox {
                        id: maximumFrequency

                        label: i18n("Maximum")
                        from: 120
                        to: 22000
                        value: EEdbSpectrum.maximumFrequency
                        decimals: 0
                        stepSize: 10
                        unit: "Hz"
                        onValueModified: (v) => {
                            EEdbSpectrum.maximumFrequency = v;
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
                        isChecked: EEdb.showNativePluginUi
                        onCheckedChanged: {
                            if (isChecked !== EEdb.showNativePluginUi)
                                EEdb.showNativePluginUi = isChecked;

                        }
                    }

                    EeSpinBox {
                        id: lv2uiUpdateFrequency

                        label: i18n("Update Frequency")
                        subtitle: i18n("Related to LV2 Plugins")
                        from: 1
                        to: 60
                        value: EEdb.lv2uiUpdateFrequency
                        decimals: 0
                        stepSize: 1
                        unit: "Hz"
                        enabled: EEdb.showNativePluginUi
                        onValueModified: (v) => {
                            EEdb.lv2uiUpdateFrequency = v;
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
