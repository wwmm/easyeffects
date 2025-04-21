var minimumLinearLevel = 0.00001;
var minimumDecibelLevel = -100.0;

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

function clamp(num, min, max) { return Math.min(Math.max(num, min), max); }

function regExpEscape(str) {
    return str.replace(/[\\/^$*+?.()|[\]{}\-]/g, '\\$&');
}

function linearTodb(value) {
    if (value >= minimumLinearLevel) {
        return 20.0 * Math.log10(value);
    }

    return minimumDecibelLevel;
}

function printObjectProperties(obj) {
    for (var prop in obj) {
        print(prop += " (" + typeof (obj[prop]) + ") = " + obj[prop]);
    }
}
