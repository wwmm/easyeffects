import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Dialog {
    id: preMixDialog

    required property var pluginDB

    title: i18n("Pre-Mix") // qmllint disable

    GridLayout {
        id: gridLayout

        columns: 2
        uniformCellWidths: true

        EeSpinBox {
            label: i18n("Input to sidechain") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("inputToSidechain")
            to: preMixDialog.pluginDB.getMaxValue("inputToSidechain")
            value: preMixDialog.pluginDB.inputToSidechain
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.inputToSidechain = v;
            }
        }

        EeSpinBox {
            label: i18n("Input to link") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("inputToLink")
            to: preMixDialog.pluginDB.getMaxValue("inputToLink")
            value: preMixDialog.pluginDB.inputToLink
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.inputToLink = v;
            }
        }

        EeSpinBox {
            label: i18n("Sidechain to input") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("sidechainToInput")
            to: preMixDialog.pluginDB.getMaxValue("sidechainToInput")
            value: preMixDialog.pluginDB.sidechainToInput
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.sidechainToInput = v;
            }
        }

        EeSpinBox {
            label: i18n("Sidechain to link") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("sidechainToLink")
            to: preMixDialog.pluginDB.getMaxValue("sidechainToLink")
            value: preMixDialog.pluginDB.sidechainToLink
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.sidechainToLink = v;
            }
        }

        EeSpinBox {
            label: i18n("Link to sidechain") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("linkToSidechain")
            to: preMixDialog.pluginDB.getMaxValue("linkToSidechain")
            value: preMixDialog.pluginDB.linkToSidechain
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.linkToSidechain = v;
            }
        }

        EeSpinBox {
            label: i18n("Link to input") // qmllint disable
            labelAbove: true
            spinboxLayoutFillWidth: true
            from: preMixDialog.pluginDB.getMinValue("linkToInput")
            to: preMixDialog.pluginDB.getMaxValue("linkToInput")
            value: preMixDialog.pluginDB.linkToInput
            decimals: 2
            stepSize: 0.01
            unit: Units.dB
            minusInfinityMode: true
            onValueModified: v => {
                preMixDialog.pluginDB.linkToInput = v;
            }
        }
    }
}
