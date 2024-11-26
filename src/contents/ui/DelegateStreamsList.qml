import EEpw
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    required property bool connected
    required property bool isBlocklisted
    required property bool mute
    required property int id
    required property int serial
    required property int nVolumeChannels
    required property real volume
    required property string name
    required property string appIconName
    required property string mediaName
    required property string mediaClass
    required property string state
    required property string format
    required property string rate
    required property string latency

    contentItem: Item {
        implicitWidth: delegateLayout.implicitWidth
        implicitHeight: delegateLayout.implicitHeight

        GridLayout {
            id: delegateLayout

            rowSpacing: Kirigami.Units.largeSpacing
            columnSpacing: Kirigami.Units.largeSpacing
            columns: appWindow.wideScreen ? 3 : 1
            rows: appWindow.wideScreen ? 2 : 3

            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
            }

            Kirigami.Icon {
                Layout.fillHeight: true
                source: appIconName
                fallback: "folder-sound-symbolic"
            }

            ColumnLayout {
                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 2
                    text: name
                }

                Controls.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    text: mediaName
                    color: Kirigami.Theme.disabledTextColor
                }

                RowLayout {
                    Controls.Label {
                        wrapMode: Text.WordWrap
                        text: state + " · " + format + " · " + rate + " · " + nVolumeChannels + i18n(" channels") + " · " + latency
                        color: Kirigami.Theme.disabledTextColor
                    }

                }

            }

            ColumnLayout {
                Controls.CheckBox {
                    text: i18n("Enable")
                    checked: connected
                    onCheckedChanged: {
                        if (checked == true && !isBlocklisted) {
                            if (mediaClass === "Stream/Output/Audio")
                                EEpwManager.connectStreamOutput(id);
                            else if (mediaClass === "Stream/Input/Audio")
                                EEpwManager.connectStreamInput(id);
                        } else if (checked == false || isBlocklisted) {
                            EEpwManager.disconnectStream(id);
                        }
                    }
                }

                Controls.CheckBox {
                    text: i18n("Exclude")
                    checked: false
                }

            }

            RowLayout {
                Layout.columnSpan: appWindow.wideScreen ? 3 : 1

                Controls.Button {
                    id: muteButton

                    icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-high-symbolic"
                    checkable: true
                    checked: mute
                    onCheckedChanged: {
                        if (checked !== mute)
                            EEpwManager.setNodeMute(serial, checked);

                    }
                }

                Controls.Slider {
                    id: volumeSlider

                    function prepareVolumeValue(normalizedValue) {
                        return DB.Manager.main.useCubicVolumes === false ? normalizedValue * 100 : Math.cbrt(normalizedValue) * 100;
                    }

                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                    value: prepareVolumeValue(volume)
                    to: 100
                    stepSize: 1
                    enabled: !muteButton.checked
                    wheelEnabled: false
                    onMoved: {
                        if (value !== prepareVolumeValue(volume)) {
                            let v = value / 100;
                            v = DB.Manager.main.useCubicVolumes === false ? v : v * v * v;
                            EEpwManager.setNodeVolume(serial, nVolumeChannels, v);
                        }
                    }
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: Math.round(volumeSlider.value) + "%"
                }

            }

        }

    }

}
