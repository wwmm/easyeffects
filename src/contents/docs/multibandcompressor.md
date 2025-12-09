# Multiband Compressor

Easy Effects uses the Sidechain Multiband Compressor Stereo developed by Linux Studio Plugins. Please refer to the [Compressor](compressor.md) documentation to comprehend the basic functionality of the dynamic range compression.

## Global Options

**Band Management**  
This Compressor allows to split the input signal up to 8 bands. Each band is not attached to its strict frequency and can be controlled by completely different frequency range that can be obtained by applying Low-Cut and Hi-Cut filters to the Sidechain signal.

The first band is always enabled while the others can be activated if needed. When only the first band is enabled, the functionality is similar to the Singleband Sidechain Compressor. Otherwise the signal is split in more bands and each band is compressed individually. After the compression stage, the bands are mixed together to form the output result.

**Operating Mode**  
Determines how the input signal is split to obtain the different bands.

- **Classic** - The original signal is split using crossover filters. After the compression stage, all bands become phase-compensated using All-Pass filters.
- **Modern** - Each band is processed by a pair of dynamic shelving filters. This mode allows a better control over the gain of each band.

**Sidechain Boost**  
If enabled, it introduces a special mode for assigning the same weight for higher frequencies opposite to lower frequencies.

When disabled, the frequency band is processed 'as is', but the usual audio signal has 3 dB/octave falloff in the frequency domain and could be compared with the pink noise. So lower frequencies take more effect on the Compressor rather than higher frequencies.

On the other hand the Sidechain Boost mode allows to compensate the -3 dB/octave falloff of the signal spectrum and, even more, makes the signal spectrum growing +3 dB/octave in the almost fully audible frequency range.

**Pink** applies +3 dB/octave while **Brown** applies +6 dB/octave Sidechain Boost. Each of them can use bilinear-transformed (BT) or matched-transformed (MT) shelving filter.

**Sidechain Source**  
If the External Sidechain is enabled inside at least one band, this combobox allows to select the input device as source.

**Dry Level**  
Amount of unprocessed signal mixed in the output.

**Wet Level**  
Amount of processed signal mixed in the output.

## Band Options

**Band Start**  
Allows to change the lower end split frequency of the selected band. This value is assigned to the Band End of the previous enabled band. It can be modified for all bands except the first one, which is always enabled and its value is 0 Hz.

**Band End**  
Specify the upper end split frequency of the selected band. It cannot be directly modified and assumes the same value of the Band Start of the next enabled band. For the last enabled band it is always 24.000 Hz.

**Compression Mode**  
- **Downward** - It's aimed to decrease the gain of the signal above the Threshold.
- **Upward** - It's aimed to increase the gain of the signal below the Threshold.
- **Boosting** - It's aimed to increase the gain of the signal below the Threshold by a specific amount.

**External Sidechain**  
The Sidechain is an external source took by a specific input device (typically a microphone).

**Band Bypass**  
If enabled, the selected band is not affected by the compression stage.

**Solo**  
Turns on the Solo mode to the selected band by applying -36 dB gain to the other non-soloing bands.

**Mute**  
Turns on the Mute mode applying -36 dB gain to to the selected band.

**Attack Time**  
The length of time it takes to apply roughly two-thirds of the targeted amount of compression ratio to the uncompressed band signal.

**Release Time**  
The length of time it takes to restore roughly two-thirds of the reduced gain (in Downward mode) or increased gain (in Upward/Boosting mode) to the compressed band signal.

**Attack Threshold**  
The target level around which the compression is applied (the range depends by the Knee).

**Release Threshold**  
Sets up the Threshold of the Release Time, calculated by summing the Release Threshold to the Attack Threshold.

If the Sidechain level is above that Threshold, the compressor uses the Release Time for the releasing stage. Otherwise the Attack Time is used in place of Release Time.

For example, with -10 dB Attack Threshold and -60 dB Release Threshold, if the Sidechain is above `-10 + (-60) = -70 dB`, the Release Time is used for the gain restoration. If the Sidechain is below -70 dB, the Attack Time is used for the gain restoration.

**Ratio**  
The amount of attenuation (in Downward mode) or amplification (in Upward/Boosting mode) that will be applied to the signal.

For example, when the Ratio is 2 in Downward mode and the Sidechain rises above the Threshold by 10 dB, the signal would be ideally reduced by 5 dB (10/2 dB). In practice this behavior mostly depends on how the Multiband Compressor is designed and configured.

**Knee**  
The range over which the Compressor switches from no compression to almost the full ratio compression (the Threshold usually sits at the center of this transition zone).

**Makeup**  
The gain to apply after the compression stage.

## Band Sidechain Options

**Mode**  
Determines how the Sidechain of the selected band is evaluated for the compression stage.

- **Peak** - The Compressor reacts according to the peaks.
- **RMS** - The Compressor reacts according to the average loudness measured by the root mean square.
- **Low-Pass** - The Compressor reacts according to the signal processed by a Low-Pass filter.
- **Uniform** - The Compressor reacts according to the loudness measured by the average of the absolute amplitude.

**Source**  
Determines which part of the Sidechain is taken into account for the compression stage.

- **Middle** - The sum of left and right channels.
- **Side** - The difference between left and right channels.
- **Left** - Only left channel is used.
- **Right** - Only right channel is used.
- **Min** - The absolute minimum value is taken from stereo input.
- **Max** - The absolute maximum value is taken from stereo input.

**Low-Cut Filter**  
Enables a custom Low-Cut Filter for the selected band.

**Low-Cut Frequency**  
Sets the cut-off frequency of the custom Low-Cut filter. If it is disabled, the default Low-Cut filter assumes internally the Band Start frequency as the cut-off frequency.

**Hight-Cut Filter**  
Enables a custom High-Cut Filter for the selected band.

**Hight-Cut Frequency**  
Sets the cut-off frequency of the custom High-Cut filter. If it is disabled, the default High-Cut filter assumes internally the Band End frequency as the cut-off frequency.

**PreAmp**  
Gain applied to the Sidechain signal of the selected band.

**Reactivity**  
The time that defines the number of samples used to process the Sidechain in RMS, Uniform and Low-Pass modes. Higher the value, more smooth the compression.

**Lookahead**  
The band signal to compress is delayed by this amount of time, so that the compression will be applied earlier than it would be otherwise.

Each band can have different Lookahead values. To avoid phase distortions in the mixing stage, all the bands are automatically delayed for an individually calculated period of time.

**Boost Amount**  
Maximum gain amplification to apply in Boosting Mode.

**Boost Threshold**  
The Threshold below which a constant amplification will be applied to the band signal in Upward Mode (it prevents from applying infinite amplification to very quiet signals).

## References

- [Wikipedia Dynamic Range Compression](https://en.wikipedia.org/wiki/Dynamic_range_compression)
- [LSP Sidechain Multiband Compressor Stereo](https://lsp-plug.in/?page=manuals&section=sc_mb_compressor_stereo)
- [Black Ghost Audio - The Ultimate Guide to Compression](https://www.blackghostaudio.com/blog/the-ultimate-guide-to-compression)
- [Attack Magazine - Demolishing The Myths of Compression](https://www.attackmagazine.com/features/columns/gregory-scott-demolishing-the-myths-of-compression/)
