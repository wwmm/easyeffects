import EEdbm
import EEpw
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.AbstractCard {

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
                source: model.appIconName
                fallback: "folder-sound-symbolic"
            }

            ColumnLayout {
                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 2
                    text: model.name
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
                        text: model.state + " · " + model.format + " · " + model.rate + " · " + model.nVolumeChannels + i18n(" channels") + " · " + model.latency
                        color: Kirigami.Theme.disabledTextColor
                    }

                }

            }

            ColumnLayout {
                Controls.CheckBox {
                    text: i18n("Enable")
                    checked: model.connected
                    onCheckedChanged: {
                        if (checked == true && !model.is_blocklisted) {
                            if (pageType === 0)
                                EEpwManager.connectStreamOutput(model.id);
                            else
                                EEpwManager.connectStreamInput(model.id);
                        } else if (checked == false || model.is_blocklisted) {
                            EEpwManager.disconnectStream(model.id);
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
                    checked: model.mute
                    onCheckedChanged: {
                        if (checked !== model.mute)
                            EEpwManager.setNodeMute(model.serial, checked);

                    }
                }

                Controls.Slider {
                    id: volumeSlider

                    function prepareVolumeValue(normalizedValue) {
                        return EEdbm.main.useCubicVolumes === false ? normalizedValue * 100 : Math.cbrt(normalizedValue) * 100;
                    }

                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                    value: prepareVolumeValue(model.volume)
                    to: 100
                    stepSize: 1
                    enabled: !muteButton.checked
                    wheelEnabled: false
                    onMoved: {
                        if (value !== prepareVolumeValue(model.volume)) {
                            let v = value / 100;
                            v = EEdbm.main.useCubicVolumes === false ? v : v * v * v;
                            EEpwManager.setNodeVolume(model.serial, model.nVolumeChannels, v);
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
