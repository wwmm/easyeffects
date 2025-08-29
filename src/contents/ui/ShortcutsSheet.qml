import QtQuick.Controls as Controls
import QtQuick.Layouts
import ee.database as DB
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: appWindow.header.height + Kirigami.Units.gridUnit
    implicitWidth: Math.min(gridLayout.implicitWidth, appWindow.width * 0.8) + 4 * Kirigami.Units.iconSizes.large

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
                text: i18n("Show Help")
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
                text: i18n("Toggle Fullscreen")
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
                text: i18n("Toggle Global Bypass")
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
                text: i18n("Close the Window")
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
                text: i18n("Terminate Easy Effects Service")
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
                text: i18n("Can Be Used on Numeric Controls")
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
                text: i18n("Can Be Used on Numeric Controls")
            }
        }

        RowLayout {
            Kirigami.Icon {
                source: "configure-shortcuts-symbolic"
            }

            Kirigami.Heading {
                Layout.alignment: Qt.AlignLeft
                text: i18n("Global Shortcuts")
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
                text: i18n("Toggle Global Bypass")
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
            text: i18n("Shortcuts")
        }
    }
}
