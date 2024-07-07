import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: widgetRoot

    Component {
        id: listDelegate

        Kirigami.AbstractCard {

            contentItem: Item {
                implicitWidth: delegateLayout.implicitWidth
                implicitHeight: delegateLayout.implicitHeight

                GridLayout {
                    id: delegateLayout

                    rowSpacing: Kirigami.Units.largeSpacing
                    columnSpacing: Kirigami.Units.largeSpacing
                    columns: root.wideScreen ? 4 : 2

                    anchors {
                        left: parent.left
                        top: parent.top
                        right: parent.right
                    }

                    ColumnLayout {
                        Controls.TextField {
                            Layout.fillWidth: true
                            placeholderText: i18n("Name")
                            text: name
                            wrapMode: TextInput.WrapAnywhere
                            onTextEdited: model.name = this.text
                        }

                        Kirigami.Separator {
                            Layout.fillWidth: true
                            visible: true
                        }

                        Controls.TextField {
                            Layout.fillWidth: true
                            placeholderText: i18n("Value")
                            text: value
                            wrapMode: TextInput.WrapAnywhere
                            onTextEdited: model.value = this.text
                        }

                    }

                    ColumnLayout {
                        Controls.Button {
                            onClicked: CppModelEnvVars.remove(index)
                            Layout.alignment: Qt.AlignRight
                            icon.name: "delete"
                        }

                    }

                }

            }

        }

    }

    Kirigami.CardsListView {
        id: envVarListView

        delegate: listDelegate
    }

}
