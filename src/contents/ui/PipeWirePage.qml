import EEpw
import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: pwPage

    padding: 0
    Component.onCompleted: {
        panelModel.append({
            "title": i18n("General"),
            "icon": "documentinfo-symbolic"
        });
        panelModel.append({
            "title": i18n("Preset Autoloading"),
            "icon": "bookmarks-symbolic"
        });
        panelModel.append({
            "title": i18n("Modules"),
            "icon": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Clients"),
            "icon": "code-block-symbolic"
        });
        panelModel.append({
            "title": i18n("Test Signal"),
            "icon": "waveform-symbolic"
        });
    }

    Component {
        id: generalPage

        FormCard.FormCardPage {
            FormCard.FormHeader {
                title: i18n("Device Management")
            }

            FormCard.FormSectionText {
                text: i18n("It's recommended to NOT set Easy Effects Sink/Source as Default Device in external applications (e.g. Gnome Settings and Plasma System Settings)")
            }

            FormCard.FormHeader {
                title: i18n("Server Information")
            }

            FormCard.FormCard {
                FormCard.FormTextDelegate {
                    text: i18n("Header Version")
                    description: EEpwManager.headerVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Library Version")
                    description: EEpwManager.libraryVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Runtime Version")
                    description: EEpwManager.runtimeVersion
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Sampling Rate")
                    description: EEpwManager.defaultClockRate + " Hz"
                }

                FormCard.FormTextDelegate {
                    text: i18n("Minimum Quantum")
                    description: EEpwManager.defaultMinQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Maximum Quantum")
                    description: EEpwManager.defaultMaxQuantum
                }

                FormCard.FormTextDelegate {
                    text: i18n("Default Quantum")
                    description: EEpwManager.defaultQuantum
                }

            }

        }

    }

    Component {
        id: modulesPage

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: modulesListView

                clip: true
                reuseItems: true
                model: ModelModules

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: modulesListView.count === 0
                    text: i18n("No Modules")
                }

                delegate: DelegateModulesList {
                }

            }

        }

    }

    Component {
        id: clientsPage

        Kirigami.ScrollablePage {
            Kirigami.CardsListView {
                id: clientsListView

                clip: true
                reuseItems: true
                model: ModelClients

                Kirigami.PlaceholderMessage {
                    anchors.centerIn: parent
                    width: parent.width - (Kirigami.Units.largeSpacing * 4)
                    visible: clientsListView.count === 0
                    text: i18n("No Clients")
                }

                delegate: DelegateClientsList {
                }

            }

        }

    }

    GridLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        columns: 3
        rows: 1
        columnSpacing: 0

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }

        ListView {
            id: panelListView

            Layout.fillHeight: true
            Layout.minimumWidth: Math.round(pwPage.width * 0.15)
            clip: true
            reuseItems: true

            model: ListModel {
                id: panelModel
            }

            delegate: Controls.ItemDelegate {
                id: listItemDelegate

                property int elide: Text.ElideRight

                width: parent ? parent.width : implicitWidth
                hoverEnabled: true
                highlighted: ListView.isCurrentItem
                onClicked: {
                    ListView.view.currentIndex = index;
                    switch (index) {
                    case 0:
                        panelStack.replace(generalPage);
                        break;
                    case 1:
                        panelStack.replace(autoloadingPage);
                        break;
                    case 2:
                        panelStack.replace(modulesPage);
                        break;
                    case 3:
                        panelStack.replace(clientsPage);
                        break;
                    case 4:
                        panelStack.replace(testSignalPage);
                        break;
                    default:
                        console.log("pipewire page stackview: invalid index");
                    }
                }

                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

                    Kirigami.Icon {
                        source: model.icon
                        Layout.preferredWidth: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.preferredHeight: Kirigami.Units.iconSizes.sizeForLabels
                        Layout.alignment: Qt.AlignLeft
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: model.title
                        color: Kirigami.Theme.textColor
                        wrapMode: Text.WordWrap
                    }

                }

            }

        }

        Kirigami.Separator {
            Layout.fillHeight: true
            visible: true
        }

        Controls.StackView {
            id: panelStack

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: panelStack.currentItem.implicitWidth
            initialItem: generalPage
        }

    }

}
