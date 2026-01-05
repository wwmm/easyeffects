/**
 * Copyright © 2025-2026 Wellington Wallace
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

import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: control

    required property string translatedName
    required property string packageName

    Kirigami.PlaceholderMessage {
        // We need to specify the width here, otherwise the text won't be
        // wrapped on window restricted sizes.
        width: parent.width
        anchors.centerIn: parent
        text: i18n("Effect Not Available") // qmllint disable
        explanation: i18n("%1 cannot be used because %2 is not installed on the system.", translatedName, `<strong>${control.packageName}</strong>`) // qmllint disable
        icon.name: "applications-system-symbolic"
    }
}
