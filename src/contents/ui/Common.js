function isEmpty(str) {
    return (!str || str.length === 0);
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
