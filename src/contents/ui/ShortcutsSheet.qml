import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    implicitWidth: Math.min(loader.implicitWidth, appWindow.width * 0.8) + 4 * Kirigami.Units.iconSizes.large // qmllint disable

    onAboutToShow: {
        loader.active = true;
    }

    onAboutToHide: {
        loader.active = false;
    }

    Loader {
        id: loader
        active: false
        asynchronous: true
        sourceComponent: ColumnLayout {
            spacing: Kirigami.Units.gridUnit

            GridLayout {
                id: gridLayout

                columns: 2
                columnSpacing: Kirigami.Units.gridUnit

                Kirigami.Chip {
                    text: "F1"
                    closable: false
                    checkable: false
                    down: false
                    hoverEnabled: false
                    Layout.alignment: Qt.AlignRight
                }

                Controls.Label {
                    text: i18n("Show help") // qmllint disable
                }

                Kirigami.Chip {
                    text: "F11"
                    closable: false
                    checkable: false
                    down: false
                    hoverEnabled: false
                    Layout.alignment: Qt.AlignRight
                }

                Controls.Label {
                    text: i18n("Toggle fullscreen") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        checkable: false
                        down: false
                        hoverEnabled: false
                        closable: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "B"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Toggle global bypass") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        checkable: false
                        down: false
                        hoverEnabled: false
                        closable: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "W"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Close the window") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "Q"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Terminate Easy Effects service") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "Shift"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "I"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Toggle process all inputs") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "Shift"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "O"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Toggle process all outputs") // qmllint disable
                }

                Kirigami.Chip {
                    Layout.alignment: Qt.AlignRight
                    text: "PgUp"
                    closable: false
                    checkable: false
                    down: false
                    hoverEnabled: false
                }

                Controls.Label {
                    text: i18n("Can be used on numeric controls") // qmllint disable
                }

                Kirigami.Chip {
                    Layout.alignment: Qt.AlignRight
                    text: "PgDn"
                    closable: false
                    checkable: false
                    down: false
                    hoverEnabled: false
                }

                Controls.Label {
                    text: i18n("Can be used on numeric controls") // qmllint disable
                }

                Item {
                    implicitHeight: Kirigami.Units.gridUnit
                }
            }

            RowLayout {
                Kirigami.Icon {
                    source: "configure-shortcuts-symbolic"
                }

                Kirigami.Heading {
                    Layout.alignment: Qt.AlignLeft
                    text: i18n("Global Shortcuts") // qmllint disable
                }
            }

            GridLayout {
                columns: 2
                columnSpacing: Kirigami.Units.gridUnit
                enabled: DbMain.xdgGlobalShortcuts

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "Alt"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "E"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Toggle global bypass") // qmllint disable
                }

                RowLayout {
                    Layout.alignment: Qt.AlignRight

                    Kirigami.Chip {
                        text: "Ctrl"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "Alt"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }

                    Controls.Label {
                        text: "+"
                    }

                    Kirigami.Chip {
                        text: "I"
                        closable: false
                        checkable: false
                        down: false
                        hoverEnabled: false
                    }
                }

                Controls.Label {
                    text: i18n("Toggle microphone monitoring") // qmllint disable
                }

                Item {
                    implicitHeight: Kirigami.Units.gridUnit
                }
            }
        }
    }

    header: RowLayout {
        Kirigami.Icon {
            id: headerIcon

            source: "configure-shortcuts-symbolic"
        }

        Kirigami.Heading {
            Layout.fillWidth: true
            text: i18n("Shortcuts") // qmllint disable
        }
    }
}
