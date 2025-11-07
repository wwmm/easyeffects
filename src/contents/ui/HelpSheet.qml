import QtQuick
import QtQuick.Controls as Controls
import QtWebEngine
import org.kde.kirigami as Kirigami

Kirigami.OverlaySheet {
    id: control

    parent: applicationWindow().overlay // qmllint disable
    closePolicy: Controls.Popup.CloseOnEscape | Controls.Popup.CloseOnPressOutsideParent
    focus: true
    y: 0

    Loader {
        id: webLoader
        active: control.opened
        asynchronous: true
        anchors.fill: parent
        sourceComponent: WebEngineView {
            implicitHeight: appWindow.maxOverlayHeight - control.header.height
            url: "qrc:/help/index.html"
        }
    }

    header: Kirigami.ActionToolBar {
        alignment: Qt.AlignLeft
        position: Controls.ToolBar.Header
        actions: [
            Kirigami.Action {
                icon.name: "draw-arrow-back-symbolic"
                onTriggered: {
                    webLoader.item.goBack();
                }
            },
            Kirigami.Action {
                icon.name: "view-refresh-symbolic"
                onTriggered: {
                    webLoader.item.reload();
                }
            },
            Kirigami.Action {
                icon.name: "draw-arrow-forward-symbolic"
                onTriggered: {
                    webLoader.item.goForward();
                }
            }
        ]
    }
}
