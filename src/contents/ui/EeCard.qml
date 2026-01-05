/**
 * Copyright © 2025-2026 Carl Schwan <carlschwan@kde.org>
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Card {
    id: root

    property alias title: heading.text // qmllint disable
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
