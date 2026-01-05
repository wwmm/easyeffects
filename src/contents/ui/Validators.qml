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

pragma Singleton

import QtQuick

QtObject {

    //strings without `/` or `\` (max 100 chars)

    readonly property RegularExpressionValidator validFileNameRegex: RegularExpressionValidator {
        regularExpression: /^[^\\/]{1,100}$/
    }

    readonly property RegularExpressionValidator removeExtRegex: RegularExpressionValidator {
        regularExpression: /(?:\.json)+$/
    }

    /**
    * Number validation.
    *
    * The regex allows the decimal notation without the thousand
    * separator. Decimal separators are dot and comma, which will
    * work differently according to the user locale. The decimal
    * notation omitting the leading zero is also permitted.
    * For leading minus sign, we need to catch also `−` (U+2212)
    * for Suomi locale.
    *
    * The regex does not assert the start and the end of the
    * string, so it will match the number between other non-digits
    * character and, if more numbers are written, only the first
    * one is considered since the global flag is not used.
    */

    readonly property var spinBoxRegex: /[-−]?(?:[.,]\d+|\d+(?:[.,]\d+)?)/
}
