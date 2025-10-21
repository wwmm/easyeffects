import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: 0
    implicitWidth: Math.min(gridLayout.implicitWidth, appWindow.width * 0.8) + 4 * Kirigami.Units.iconSizes.large // qmllint disable

    ColumnLayout {
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
                text: i18n("Show Help") // qmllint disable
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
                text: i18n("Toggle Fullscreen") // qmllint disable
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
                text: i18n("Toggle Global Bypass") // qmllint disable
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
                text: i18n("Close the Window") // qmllint disable
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
                    text: "T"
                    closable: false
                    checkable: false
                    down: false
                    hoverEnabled: false
                }
            }

            Controls.Label {
                text: i18n("Terminate Easy Effects Service") // qmllint disable
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
                text: i18n("Toggle Process All Inputs") // qmllint disable
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
                text: i18n("Toggle Process All Outputs") // qmllint disable
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
                text: i18n("Can Be Used on Numeric Controls") // qmllint disable
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
                text: i18n("Can Be Used on Numeric Controls") // qmllint disable
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
            enabled: DB.Manager.main.xdgGlobalShortcuts

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
                text: i18n("Toggle Global Bypass") // qmllint disable
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
                text: i18n("Toggle Microphone Monitoring") // qmllint disable
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
