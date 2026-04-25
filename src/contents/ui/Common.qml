pragma Singleton
import QtQuick

QtObject {
    readonly property real minimumLinearLevel: 0.00001
    readonly property real minimumDecibelLevel: -100.0

    function isEmpty(v: string): bool {
        if (v === undefined || v === null)
            return true;

        const type = typeof v;
        if (type === "string")
            return v.length === 0;
        if (type === "number")
            return isNaN(v);
        if (type === "object")
            return v === null;
        return false;
    }

    function isNullOrUndefined(v: any): bool {
        return v === null || v === undefined;
    }

    function equalArrays(a: Array, b: Array): bool {
        if (!a || !b) {
            return false;
        }

        if (a.length !== b.length) {
            return false;
        }

        for (let i = 0; i < a.length; ++i) {
            if (a[i] !== b[i]) {
                return false;
            }
        }

        return true;
    }

    function clamp(num: real, min: real, max: real): real {
        return Math.min(Math.max(num, min), max);
    }

    function regExpEscape(str: string): string {
        // Pre-compile regex for better performance
        const regex = /[\\/^$*+?.()|[\]{}\-]/g;
        return str.replace(regex, '\\$&');
    }

    function linearTodb(value: real): real {
        let n = value;

        if (typeof value !== "number") {
            n = Number(value);

            if (isNaN(n)) {
                return minimumDecibelLevel;
            }
        }

        if (n >= minimumLinearLevel) {
            return 20.0 * Math.log10(n);
        }

        return minimumDecibelLevel;
    }

    function dbToLinear(dbValue: real): real {
        let n = dbValue;

        if (typeof dbValue !== "number") {
            n = Number(dbValue);

            if (isNaN(n)) {
                return minimumLinearLevel;
            }
        }

        if (n >= minimumDecibelLevel) {
            return Math.exp((n / 20.0) * Math.LN10);
        }

        return minimumLinearLevel;
    }

    function printObjectProperties(obj: any): void {
        if (!obj)
            return;

        for (let prop in obj) {
            const value = obj[prop];
            print(prop + " (" + typeof (value) + ") = " + value);
        }
    }

    function toLocaleLabel(num: real, decimal = 0, unit = null): string {
        let n = num;

        if (typeof num !== "number") {
            n = Number(num);

            if (isNaN(n)) {
                console.error("Cannot convert " + num + " to locale format.");
                return "";
            }
        }

        const result = n.toLocaleString(Qt.locale(), 'f', decimal);

        return unit ? result + " " + unit : result;
    }
}
