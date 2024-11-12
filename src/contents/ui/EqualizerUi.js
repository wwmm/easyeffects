const import_apo_preset = (text_file) => {
    // INTERNAL PROPERTIES
    // Apo filter class.
    class APO_Band {
        type = "";
        freq = 1000;
        gain = 0;
        quality = 1 / Math.sqrt(2);  // default in LSP APO import
    }

    // INTERNAL METHODS
    // Parse APO preamp
    const parse_apo_preamp = (line, preamp) => {
        const apo_preamp = line.match(/preamp\s*:\s*([+-]?\d+(?:\.\d+)?)\s*db/i);

        if (apo_preamp === null || apo_preamp.length !== 2) {
            return false;
        }

        const preamp_num = Number(apo_preamp[1]);

        return (isNaN(preamp_num)) ? null : preamp_num;
    }

    // Parse filter type
    const parse_apo_filter_type = (line, filter) => {
        // Look for disabled filter.
        if (line.match(/filter\s*\d*\s*:\s*off\s/i) !== null) {
            // If the APO filter is disabled, we assume the "OFF" type.
            filter.type = "OFF";

            return true;
        }

        const filter_type = line.match(/filter\s*\d*\s*:\s*on\s+([a-z]+(?:\s+(?:6|12)db)?)/i);

        if (filter_type === null || filter_type.length !== 2) {
            return false;
        }

        // Replace possible multiple whitespaces by a single space and convert to uppercase.
        filter.type = filter_type[1].replaceAll(/\s+/g, " ").toUpperCase();

        return filter.type !== "";
    }

    // Parse filter frequency.
    const parse_apo_frequency = (line, filter) => {
        const filter_freq = line.match(/fc\s+(\d+(?:,\d+)?(?:\.\d+)?)\s*hz/i);

        if (filter_freq === null || filter_freq.length !== 2) {
            return false;
        }

        // Frequency could have a comma as thousands separator
        // to be removed for the correct float conversion.
        const freq_num = Number(filter_freq[1].replaceAll(",", ""));

        if (isNaN(freq_num)) {
            return false;
        }

        filter.freq = freq_num;

        return true;
    }

    // Parse filter gain
    const parse_apo_gain = (line, filter) => {
        const filter_gain = line.match(/gain\s+([+-]?\d+(?:\.\d+)?)\s*db/i);

        if (filter_gain === null || filter_gain.length !== 2) {
            return false;
        }

        const gain_num = Number(filter_gain[1]);

        if (isNaN(gain_num)) {
            return false;
        }

        filter.gain = gain_num;

        return true;
    }

    // Parse filter quality
    const parse_apo_quality = (line, filter) => {
        const filter_q = line.match(/q\s+(\d+(?:\.\d+)?)/i);

        if (filter_q === null || filter_q.length !== 2) {
            return false;
        }

        const q_num = Number(filter_q[1]);

        if (isNaN(q_num)) {
            return false;
        }

        filter.quality = q_num;

        return true;
    }

    // Parse test file single line.
    const parse_apo_config_line = (line, filter) => {
        // Retrieve filter type.
        if (!parse_apo_filter_type(line, filter)) {
            // If we can't parse the filter type, there's something wrong in the text line,
            // so exit with false.
            return false;
        }

        // Retrieve frequency.
        // To make it more permissive, we do not exit on false here (assume default).
        parse_apo_frequency(line, filter);

        // The following has been inspired by the function
        // "para_equalizer_ui::import_rew_file(const LSPString*)"
        // inside 'lsp-plugins/src/ui/plugins/para_equalizer_ui.cpp' at
        // https://github.com/sadko4u/lsp-plugins

        // Retrieve gain and/or quality parameters based on a specific filter type.
        // Calculate frequency/quality if needed.
        // If the APO filter type is different than the ones specified below,
        // it's set as "Off" and default values are assumed since
        // it may not be supported by LSP Equalizer.
        if (filter.type === "OFF") {
            // On disabled filter state, we still try to retrieve gain and quality,
            // even if the band won't be processed by LSP equalizer.
            parse_apo_gain(line, filter);

            parse_apo_quality(line, filter);
        } else if (filter.type === "PK" || filter.type === "MODAL" || filter.type === "PEQ") {
            // Peak/Bell filter
            parse_apo_gain(line, filter);

            parse_apo_quality(line, filter);
        } else if (filter.type == "LP" || filter.type == "LPQ" || filter.type == "HP" || filter.type == "HPQ" || filter.type == "BP") {
            // Low-pass, High-pass and Band-pass filters,
            // (LSP does not import Band-pass, but we do it anyway).
            parse_apo_quality(line, filter);
        } else if (filter.type == "LS" || filter.type == "LSC" || filter.type == "HS" || filter.type == "HSC") {
            // Low-shelf and High-shelf filters (with center freq., x dB per oct.)
            parse_apo_gain(line, filter);

            // Q value is optional for these filters according to APO config documentation,
            // but LSP import function always sets it to 2/3.
            filter.quality = 2 / 3;
        } else if (filter.type == "LS 6DB") {
            // Low-shelf filter (6 dB per octave with corner freq.)
            parse_apo_gain(line, filter);

            // LSP import function sets custom freq and quality for this filter.
            filter.freq = filter.freq * 2 / 3;
            filter.quality = Math.sqrt(2) / 3;
        } else if (filter.type == "LS 12DB") {
            // Low-shelf filter (12 dB per octave with corner freq.)
            parse_apo_gain(line, filter);

            // LSP import function sets custom freq for this filter.
            filter.freq = filter.freq * 3 / 2;
        } else if (filter.type == "HS 6DB") {
            // High-shelf filter (6 dB per octave with corner freq.)
            parse_apo_gain(line, filter);

            // LSP import function sets custom freq and quality for this filter.
            filter.freq = filter.freq / (1 / Math.sqrt(2));
            filter.quality = Math.sqrt(2) / 3;
        } else if (filter.type == "HS 12DB") {
            // High-shelf filter (12 dB per octave with corner freq.)
            parse_apo_gain(line, filter);

            // LSP import function sets custom freq for this filter.
            filter.freq = filter.freq * (1 / Math.sqrt(2));
        } else if (filter.type == "NO") {
            // Notch filter
            // Q value is optional for this filter according to APO config documentation,
            // but LSP import function always sets it to 100/3.
            filter.quality = 100 / 3;
        } else if (filter.type == "AP") {
            // All-pass filter
            // Q value is mandatory for this filter according to APO config documentation,
            // but LSP import function always sets it to 0,
            // no matter which quality value the APO config has.
            filter.quality = 0;
        }

        return true;
    };

    // FUNCTION BODY
    let bands = [];
    let preamp = 0;

    const lines = text_file.match(/[^\n]+/g);
    for (const line of lines) {
        // Avoid commented lines
        if (line.match(/^[ \t]*#/) !== null) {
            continue;
        }

        let filter = new APO_Band();

        if (parse_apo_config_line(line, filter)) {
            bands.push(filter);
        } else {
            const new_preamp = parse_apo_preamp(line, preamp);

            preamp = new_preamp ?? preamp;
        }
    }

    console.log(preamp);
    console.log(bands);
};  