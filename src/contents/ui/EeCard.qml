// SPDX-FileCopyrightText: 2025 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root

    property alias title: heading.text
    default property alias content: innerLayout.children

    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    spacing: 0

    header: Kirigami.Heading {
        id: heading

        level: 2

        leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
    }

    contentItem: ColumnLayout {
        id: innerLayout

        spacing: 0
    }
}
