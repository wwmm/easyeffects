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
            return isNaN(v);
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
function clamp(num, min, max) { return Math.min(Math.max(num, min), max); }

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
 * @param {*} value The linear value.
 * @returns {number} The decibel level.
 */
function linearTodb(value) {
    if (value >= minimumLinearLevel) {
        return 20.0 * Math.log10(value);
    }

    return minimumDecibelLevel;
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
