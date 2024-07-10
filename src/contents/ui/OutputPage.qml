import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {

    header: EeChart {
        seriesType: EEdbSpectrum.spectrumShape
        useOpenGL: EEdbSpectrum.useOpenGL
    }

    footer: FooterInputOutput {
        streamsTabIconName: "multimedia-player-symbolic"
    }

}
