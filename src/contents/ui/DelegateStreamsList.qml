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
                        text: model.state + " · " + model.format + " · " + model.rate + " · " + model.nVolumeChannels + " · " + model.latency
                        color: Kirigami.Theme.disabledTextColor
                    }

                }

            }

            ColumnLayout {
                Controls.CheckBox {
                    text: i18n("Enable")
                    checked: true
                }

                Controls.CheckBox {
                    text: i18n("Exclude")
                    checked: false
                }

            }

            RowLayout {
                Layout.columnSpan: appWindow.wideScreen ? 3 : 1

                Controls.Button {
                    // onCheckedChanged: showPassiveNotification("The muted state has changed: " + checked)

                    id: muteButton

                    icon.name: checked ? "audio-volume-muted-symbolic" : "audio-volume-high-symbolic"
                    checkable: true
                    checked: model.mute
                }

                Controls.Slider {
                    id: volumeSlider

                    Layout.fillWidth: true
                    orientation: Qt.Horizontal
                    value: model.volume
                    to: 100
                    stepSize: 1
                    enabled: !muteButton.checked
                    wheelEnabled: false
                }

                Controls.Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: Math.round(volumeSlider.value) + "%"
                }

            }

        }

    }

}
