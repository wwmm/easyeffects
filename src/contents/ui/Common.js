/** @type {number} */
var minimumLinearLevel = 0.00001;
/** @type {number} */
var minimumDecibelLevel = -100.0;

/**
 * Check if a variable of any type is empty.
 * @param {*} v
 * @returns {boolean}
 */
function isEmpty(v) {
    switch (typeof v) {
    case "string":
        return v.length === 0;
    case "number":
        return Number.isNaN(v);
    case "undefined":
        return true;
    case "object":
        return v === null;
    default:
        return false;
    }
}

/**
 * Check if two given arrays are equal.
 * @param {Array} a
 * @param {Array} b
 * @returns {boolean}
 */
function equalArrays(a, b) {
    if (a.length !== b.length) {
        return false;
    }

    for (let n = 0; n < a.length; n++) {
        // We use this function in a situation where the order of the elements matter. So if the same value is on
        // another position the arrays are different

        if (a[n] !== b[n]) {
            return false;
        }
    }

    return true;
}

/**
 * Clamps a number within the range of min and max parameters.
 * @param {number} num The number to clamp.
 * @param {number} min Minimum bound.
 * @param {number} max Maximum bound.
 * @returns {number} Clamped value.
 */
function clamp(num, min, max) {
    return Math.min(Math.max(num, min), max);
}

/**
 * A simple polyfill implementation of RegExp.escape() static method which is currently not available in QML
 * Javascript engine.
 * Given a regex string as parameter, it escapes any potential regex syntax characters and returns a new string
 * that can be safely used as a literal pattern for the RegExp() constructor.
 * Mostly used for QML SearchField widgets.
 * @param {string} str Original regex.
 * @returns {string} Escaped regex.
 */
function regExpEscape(str) {
    return str.replace(/[\\/^$*+?.()|[\]{}\-]/g, '\\$&');
}

/**
 * Returns the decibel level of a linear value. If the value is lesser then minimumLinearLevel,
 * the minimumDecibelLevel is returned.
 * @param {number|string} value The linear value.
 * @returns {number} The decibel level.
 */
function linearTodb(value) {
    // Convert to number if necessary.
    const n = (typeof value === "number") ? value : Number(value);

    if (!Number.isNaN(n) && n >= minimumLinearLevel) {
        return 20.0 * Math.log10(n);
    }

    return minimumDecibelLevel;
}

/**
 * Returns the linear level of a decibel value. If the value is lesser then minimumDecibelLevel,
 * the minimumLinearLevel is returned.
 * @param {number|string} value The decibel value.
 * @returns {number} The linear level.
 */
function dbToLinear(dbValue) {
    // Convert to number if necessary.
    const n = (typeof dbValue === "number") ? dbValue : Number(dbValue);

    if (!Number.isNaN(n) && n >= minimumDecibelLevel) {
        return Math.exp((n / 20.0) * Math.LN10);
    }

    return minimumLinearLevel;
}

/**
 * A debugging function which prints the properties of a given object or any other collection type such as Array.
 * Since it uses the "in" operator, an exception can be raised if a primitive type is given as a parameter.
 * @param {*} obj
 */
function printObjectProperties(obj) {
    for (let prop in obj) {
        print(prop += " (" + typeof (obj[prop]) + ") = " + obj[prop]);
    }
}

/**
 * Converts a value to a numeric string using the current system user locale.
 * Optional decimal and unit parameters can be provided.
 * If the value cannot be converted, an empty string is returned
 * @param {number|string} num The value to convert.
 * @param {number} [decimal=0] A positive integer representing the decimal precision.
 * @param {?string} [unit=null] An optional unit string to concatenate to the converted value.
 * @returns {string} The string in locale format.
 */
function toLocaleLabel(num, decimal = 0, unit = null) {
    // Convert to number if necessary.
    const n = (typeof num === "number") ? num : Number(num);

    /**
     * Since we already have a number type, we can use `Number.isNaN()` static method
     * avoiding the type coercion which is done by `isNaN()` global function.
     * See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Number/isNaN
     */
    if (Number.isNaN(n)) {
        console.error("Cannot convert " + num + " in locale format.");
        return "";
    }

    // Sum has precedence over ternary operator, so we need parentheses.
    return n.toLocaleString(Qt.locale(), 'f', decimal) + ((unit === null) ? "" : ` ${unit}`);
}
