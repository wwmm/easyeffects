pragma ComponentBehavior: Bound
pragma Singleton
import QtQuick

QtObject {
    readonly property real minimumLinearLevel: 0.00001
    readonly property real minimumDecibelLevel: -100.0

    function isEmpty(v: string): bool {
        if (v === undefined || v === null) {
            return true;
        }

        return v.length === 0;
    }

    function isNullOrUndefined(v: var): bool {
        return v === null || v === undefined;
    }

    function equalStringArrays(a: var, b: var): bool {
        if (!a || !b) {
            return false;
        }

        const A = Array.from(a);
        const B = Array.from(b);

        if (A.length !== B.length) {
            return false;
        }

        for (let i = 0; i < A.length; i++) {
            if (A[i] !== B[i]) {
                return false;
            }
        }

        return true;
    }

    function clamp(num: real, min: real, max: real): real {
        return Math.min(Math.max(num, min), max);
    }

    function regExpEscape(str: string): string {
        const regex = /[\\/^$*+?.()|[\]{}\-]/g;
        return str.replace(regex, '\\$&');
    }

    function linearTodb(value: real): real {
        let n = value;

        if (n >= minimumLinearLevel) {
            return 20.0 * Math.log10(n);
        }

        return minimumDecibelLevel;
    }

    function dbToLinear(dbValue: real): real {
        let n = dbValue;

        if (n >= minimumDecibelLevel) {
            return Math.exp((n / 20.0) * Math.LN10);
        }

        return minimumLinearLevel;
    }

    function printObjectProperties(obj: var): void {
        if (!obj)
            return;

        for (let prop in obj) {
            const value = obj[prop];
            print(prop + " (" + typeof (value) + ") = " + value);
        }
    }

    function toLocaleLabel(num: real, decimal: int, unit: string): string {
        let n = num;

        const result = n.toLocaleString(Qt.locale(), 'f', decimal);

        return unit ? result + " " + unit : result;
    }
}
