/**
 * Copyright © 2025-2026 Björn Feber <bfeber@protonmail.com>
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
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

/*!
  \qmltype EeListSectionHeader

  \brief A section delegate for the primitive ListView component.

  It's intended to make all listviews look coherent.

  Any additional content items will be positioned in a row at the trailing side
  of this component.

  Example usage:
  \code
  import QtQuick
  import QtQuick.Controls as QQC2
  import org.kde.kirigami as Kirigami

  ListView {
      section.delegate: Kirigami.ListSectionHeader {
          text: section

          QQC2.Button {
              text: "Button 1"
          }
          QQC2.Button {
              text: "Button 2"
          }
      }
  }
  \endcode
 */
QQC2.ItemDelegate {
    id: listSection

    required property bool compact

    default property alias _contents: rowLayout.data

    hoverEnabled: false

    activeFocusOnTab: false

    icon {
        width: Kirigami.Units.iconSizes.smallMedium
        height: Kirigami.Units.iconSizes.smallMedium
    }

    // we do not need a background
    background: Item {}

    topPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

    Accessible.role: Accessible.Heading

    contentItem: RowLayout {
        id: rowLayout
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Icon {
            Layout.alignment: DbMain.collapsePluginsList ? Qt.AlignVCenter | Qt.AlignHCenter : Qt.AlignVCenter
            implicitWidth: listSection.icon.width
            implicitHeight: listSection.icon.height
            color: listSection.icon.color
            source: listSection.icon.name.length > 0 ? listSection.icon.name : listSection.icon.source
            visible: valid
        }
        Kirigami.Heading {
            Layout.maximumWidth: rowLayout.width
            Layout.alignment: Qt.AlignVCenter

            opacity: 0.75
            level: 5
            type: Kirigami.Heading.Primary
            text: listSection.text
            elide: Text.ElideRight
            visible: !listSection.compact

            // we override the Primary type's font weight (DemiBold) for Bold for contrast with small text
            font.weight: Font.Bold

            Accessible.ignored: true
        }

        Kirigami.Separator {
            visible: !listSection.compact

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignVCenter
            Accessible.ignored: true
        }
    }
}
