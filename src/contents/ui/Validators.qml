pragma Singleton

import QtQuick

QtObject {
    readonly property RegularExpressionValidator numberValidator: RegularExpressionValidator {
        /**
         * We need also `−` (U+2212) minus sign for Suomi localization.
         * Some other localizations may also use `＋` (U+FF0B) plus sign, so
         * let's allow it too for a more robust input handling. See #4417.
         */
        regularExpression: /^[-−+＋]?(?:inf|∞|\d+(?:[.,]\d*)?(?:[eE][-+]?\d+)?)$/
    }

    readonly property RegularExpressionValidator validFileNameRegex: RegularExpressionValidator {
        regularExpression: /^[^\\/]{1,100}$/ //strings without `/` or `\` (max 100 chars)
    }

    readonly property RegularExpressionValidator removeExtRegex: RegularExpressionValidator {
        regularExpression: /(?:\.json)+$/
    }
}
