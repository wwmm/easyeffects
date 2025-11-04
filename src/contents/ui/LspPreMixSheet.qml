import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.OverlaySheet {
    id: preMixSheet

    required property var pluginDB

    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    title: i18n("Pre-Mix") // qmllint disable
    y: 0

    implicitWidth: Math.min(gridLayout.implicitWidth, preMixSheet.parent.width * 0.8)// qmllint disable

    ColumnLayout {

        FormCard.FormCard {
            GridLayout {
                id: gridLayout

                columns: 2
                uniformCellWidths: true
                Layout.alignment: Qt.AlignTop

                EeSpinBox {
                    label: i18n("Input to sidechain") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("inputToSidechain")
                    to: preMixSheet.pluginDB.getMaxValue("inputToSidechain")
                    value: preMixSheet.pluginDB.inputToSidechain
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.inputToSidechain = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Input to link") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("inputToLink")
                    to: preMixSheet.pluginDB.getMaxValue("inputToLink")
                    value: preMixSheet.pluginDB.inputToLink
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.inputToLink = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Sidechain to input") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("sidechainToInput")
                    to: preMixSheet.pluginDB.getMaxValue("sidechainToInput")
                    value: preMixSheet.pluginDB.sidechainToInput
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.sidechainToInput = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Sidechain to link") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("sidechainToLink")
                    to: preMixSheet.pluginDB.getMaxValue("sidechainToLink")
                    value: preMixSheet.pluginDB.sidechainToLink
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.sidechainToLink = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Link to sidechain") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("linkToSidechain")
                    to: preMixSheet.pluginDB.getMaxValue("linkToSidechain")
                    value: preMixSheet.pluginDB.linkToSidechain
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.linkToSidechain = v;
                    }
                }

                EeSpinBox {
                    label: i18n("Link to input") // qmllint disable
                    labelAbove: true
                    spinboxLayoutFillWidth: true
                    from: preMixSheet.pluginDB.getMinValue("linkToInput")
                    to: preMixSheet.pluginDB.getMaxValue("linkToInput")
                    value: preMixSheet.pluginDB.linkToInput
                    decimals: 2
                    stepSize: 0.01
                    unit: "dB"
                    minusInfinityMode: true
                    onValueModified: v => {
                        preMixSheet.pluginDB.linkToInput = v;
                    }
                }
            }
        }
    }
}
