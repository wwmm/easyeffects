import QtQuick.Layouts
import org.kde.kirigami as Kirigami

ColumnLayout {
    Kirigami.AboutPage {
        Layout.fillHeight: true
        Layout.fillWidth: true

        title: i18nc("@action:button", "About")
        aboutData: {
            "displayName": applicationName,
            "productName": "easyeffects",
            "componentName": "easyeffects",
            "shortDescription": i18n("Simple audio effects") // qmllint disable
            ,
            "homepage": "https://github.com/wwmm/easyeffects",
            "bugAddress": "https://github.com/wwmm/easyeffects/issues",
            "donateUrl": "https://github.com/wwmm/easyeffects",
            "getInvolvedUrl": "https://github.com/wwmm/easyeffects",
            "version": projectVersion// qmllint disable
            ,
            "programLogo": applicationId,
            "otherText": "",
            "authors": [
                {
                    "name": "Wellington Wallace",
                    "task": i18nc("@info:credit", "Developer") // qmllint disable
                    ,
                    "emailAddress": "wellingtonwallace@gmail.com",
                    "webAddress": "",
                    "ocsUsername": ""
                }
            ],
            "credits": [],
            "translators": [],
            "licenses": [
                {
                    "name": "GPL v3",
                    "text": "https://github.com/wwmm/easyeffects/blob/master/LICENSE",
                    "spdx": "GPL-3.0"
                }
            ],
            "copyrightStatement": "© 2017-2025 EasyEffects Team",
            "desktopFileName": applicationId + ".desktop"
        }
    }
}
