import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import ee.pipewire as PW
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {
    required property var model
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

    visible: {
        if (!isBlocklisted)
            return true;

        if (mediaClass === "Stream/Output/Audio")
            return DB.Manager.streamOutputs.showBlocklistedApps;
        else if (mediaClass === "Stream/Input/Audio")
            return DB.Manager.streamInputs.showBlocklistedApps;
    }

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
                        text: (state === "" ? "" : `${state} · `) + `${format} · ${rate} · ${nVolumeChannels} ` + i18n("channels") + ` · ${latency}`
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
                                PW.Manager.connectStreamOutput(id);
                            else if (mediaClass === "Stream/Input/Audio")
                                PW.Manager.connectStreamInput(id);
                        } else if (checked == false || isBlocklisted) {
                            PW.Manager.disconnectStream(id);
                        }
                    }
                }

                Controls.CheckBox {
                    text: i18n("Exclude")
                    checked: isBlocklisted
                    onCheckedChanged: {
                        if (model.isBlocklisted !== checked)
                            model.isBlocklisted = checked;
                    }
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
                            PW.Manager.setNodeMute(serial, checked);
                    }
                }

                Controls.Slider {
                    id: volumeSlider

                    function prepareVolumeValue(normalizedValue) {
                        const v = DB.Manager.main.useCubicVolumes === false ? normalizedValue : Math.cbrt(normalizedValue);
                        return v * 100;
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
                            PW.Manager.setNodeVolume(serial, nVolumeChannels, v);
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
