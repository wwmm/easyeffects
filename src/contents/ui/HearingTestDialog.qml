pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.ui
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: dialog

    required property var equalizer
    readonly property var frequencies: [250, 500, 1000, 2000, 4000, 8000]
    property var leftThresholds: []
    property var rightThresholds: []
    property int measurement: 0
    property bool started: false
    property bool complete: false
    property bool played: false
    property string errorText: ""
    readonly property int band: measurement % frequencies.length
    readonly property int channel: measurement < frequencies.length ? 0 : 1

    title: i18n("Hearing Test")
    standardButtons: Kirigami.Dialog.NoButton
    parent: applicationWindow().overlay // qmllint disable
    implicitWidth: Math.min(Kirigami.Units.gridUnit * 30, appWindow.width * 0.8) // qmllint disable
    maximumHeight: Controls.ApplicationWindow.window.height * 0.8
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing
    topPadding: Kirigami.Units.largeSpacing
    bottomPadding: Kirigami.Units.largeSpacing * 2
    anchors.centerIn: parent

    header: Item {
        width: parent.width
        implicitHeight: titleLabel.implicitHeight + Kirigami.Units.largeSpacing * 2

        Kirigami.Heading {
            id: titleLabel

            anchors.centerIn: parent
            text: dialog.title
            horizontalAlignment: Text.AlignHCenter
        }

        Controls.ToolButton {
            text: i18nc("@action:button", "Close")
            icon.name: "dialog-close-symbolic"
            display: Controls.ToolButton.IconOnly
            onClicked: dialog.close()
            anchors {
                margins: Kirigami.Units.smallSpacing
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
        }
    }

    function finish() {
        if (started) {
            started = false;
            TestTone.endTest();
        }
    }

    onOpened: {
        leftThresholds = [];
        rightThresholds = [];
        measurement = 0;
        complete = false;
        played = false;
        errorText = "";
        headphones.checked = false;
        level.value = -60;
    }
    onClosed: finish()
    Component.onDestruction: finish()

    Connections {
        target: TestTone
        function onTestingChanged() {
            if (dialog.started && !TestTone.testing) {
                dialog.started = false;
                dialog.complete = false;
                dialog.errorText = i18n("The output device changed. Restart the test with the same headphones and volume.");
            }
        }
    }

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        Controls.Label {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            wrapMode: Text.WordWrap
            text: i18n("Use headphones in a quiet room. Pause other audio and keep the system volume fixed at a comfortable level. Adjust each tone until it is barely audible, then record the threshold. This uncalibrated listening test is not a medical hearing assessment.")
        }

        Controls.CheckBox {
            id: headphones
            Layout.alignment: Qt.AlignHCenter
            visible: !dialog.started && !dialog.complete
            text: i18n("I am using headphones")
        }

        Controls.Label {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            visible: dialog.errorText.length > 0
            wrapMode: Text.WordWrap
            text: dialog.errorText
            color: Kirigami.Theme.negativeTextColor
        }

        Controls.Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Kirigami.Units.mediumSpacing
            visible: !dialog.started && !dialog.complete
            enabled: headphones.checked
            text: i18n("Start test")
            onClicked: {
                if (TestTone.beginTest()) {
                    dialog.measurement = 0;
                    dialog.leftThresholds = [];
                    dialog.rightThresholds = [];
                    dialog.started = true;
                    dialog.played = false;
                    dialog.errorText = "";
                    level.value = -60;
                } else {
                    dialog.errorText = i18n("Select an available stereo output device and disable the regular test signals before starting.");
                }
            }
        }

        Controls.Label {
            Layout.margins: Kirigami.Units.largeSpacing
            visible: dialog.started && !dialog.complete
            text: i18nc("Left/right ear: frequency Hz (number of total)", "%1 ear: %2 Hz (%3 of %4)", dialog.channel === 0 ? i18nc("Channel", "Left") : i18nc("Channel", "Right"), dialog.frequencies[dialog.band], dialog.measurement + 1, dialog.frequencies.length * 2)
        }

        EeSpinBox {
            id: level
            visible: dialog.started && !dialog.complete
            label: i18n("Tone level")
            from: -80
            to: -20
            value: -60
            decimals: 0
            stepSize: 1
            unit: Units.dB
            onValueModified: v => {
                level.value = v;
                TestTone.stopTone();
                dialog.played = false;
            }
        }

        RowLayout {
            visible: dialog.started && !dialog.complete
            Layout.alignment: Qt.AlignHCenter

            Controls.Button {
                text: i18n("Play tone")
                onClicked: {
                    dialog.played = TestTone.playTone(dialog.frequencies[dialog.band], dialog.channel, level.value);
                }
            }

            Controls.Button {
                text: i18n("Record threshold")
                enabled: dialog.played
                onClicked: {
                    TestTone.stopTone();
                    if (dialog.channel === 0)
                        dialog.leftThresholds.push(level.value);
                    else
                        dialog.rightThresholds.push(level.value);
                    dialog.measurement++;
                    dialog.played = false;
                    level.value = -60;
                    if (dialog.measurement === dialog.frequencies.length * 2)
                        dialog.complete = true;
                }
            }
        }

        Controls.Label {
            visible: dialog.complete
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            wrapMode: Text.WordWrap
            text: i18n("The test is complete. Applying the profile replaces this equalizer's settings with separate corrections for each ear, limited to 6 dB per band, and lowers input gain to preserve headroom. Save a preset to keep the result for these headphones.")
        }

        Repeater {
            model: dialog.complete ? dialog.frequencies.length : 0
            delegate: Controls.Label {
                required property int index
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.leftMargin: Kirigami.Units.largeSpacing
                text: i18n("%1 Hz: left %2 dB, right %3 dB", dialog.frequencies[index], dialog.leftThresholds[index], dialog.rightThresholds[index])
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: Kirigami.Units.mediumSpacing

            Controls.Button {
                visible: dialog.complete
                text: i18n("Apply personalized EQ")
                onClicked: {
                    if (dialog.equalizer.applyHearingProfile(dialog.leftThresholds, dialog.rightThresholds))
                        dialog.close();
                    else
                        dialog.errorText = i18n("Could not apply the hearing profile.");
                }
            }

            Controls.Button {
                text: i18n("Cancel")
                onClicked: dialog.close()
            }
        }
    }
}
