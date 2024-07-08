import EEdb
import QtQml
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

Kirigami.OverlaySheet {
    implicitWidth: Kirigami.Units.gridUnit * 30
    implicitHeight: appWindow.height * 0.7
    title: i18n("Preferences")

    Component {
        id: mainPage

        Kirigami.Page {
            FormCard.FormCard {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                FormCard.FormButtonDelegate {
                    id: serviceButton

                    icon.name: "services-symbolic"
                    text: i18n("Service")
                    onClicked: {
                        stack.replace(servicePage);
                    }
                }

                FormCard.FormButtonDelegate {
                    id: audioButton

                    icon.name: "folder-sound-symbolic"
                    text: i18n("Audio")
                }

                FormCard.FormButtonDelegate {
                    id: spectrumButton

                    icon.name: "folder-chart-symbolic"
                    text: i18n("Spectrum Analyzer")
                }

                FormCard.FormButtonDelegate {
                    id: experimentalButton

                    icon.name: "emblem-warning"
                    text: i18n("Experimental Features")
                }

            }

        }

    }

    Component {
        id: servicePage

        Kirigami.Page {
            FormCard.FormCard {
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    right: parent.right
                    rightMargin: Kirigami.Units.smallSpacing
                }

                EESwitch {
                    id: addRandom

                    label: i18n("Launch Service at System Startup")
                    // isChecked: EEdb.addRandom
                    onCheckedChanged: {
                    }
                }

                EESwitch {
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

    Controls.StackView {
        id: stack

        initialItem: mainPage

        anchors {
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: Kirigami.Units.smallSpacing
        }

    }

    header: RowLayout {
        Kirigami.Icon {
            source: "gtk-preferences-symbolic"
        }

        Kirigami.Heading {
            Layout.fillWidth: true
            text: i18n("Preferences")
        }

    }

}
