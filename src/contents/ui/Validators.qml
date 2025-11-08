pragma Singleton

import QtQuick

QtObject {
    readonly property RegularExpressionValidator numberValidator: RegularExpressionValidator {
        regularExpression: /^[-+]?(?:inf|∞|\d+(?:[.,]\d*)?(?:[eE][-+]?\d+)?)$/
    }

    readonly property RegularExpressionValidator validFileNameRegex: RegularExpressionValidator {
        regularExpression: /^[^\\/]{1,100}$/ //strings without `/` or `\` (max 100 chars)
    }

    readonly property RegularExpressionValidator removeExtRegex: RegularExpressionValidator {
        regularExpression: /(?:\.json)+$/
    }
}
