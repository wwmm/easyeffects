# Multiband Gate

Easy Effects uses the Multiband Gate Stereo developed by Calf Studio Gear. Please refer to the [Gate](gate.md) documentation to comprehend the basic functionality of the gating process.

## Global Options

**Band Management**  
This Gate allows to split the input signal up to 8 bands. Each band is not attached to its strict frequency and can be controlled by completely different frequency range that can be obtained by applying Low-Cut and Hi-Cut filters to the Sidechain signal.

The first band is always enabled while the others can be activated if needed. When only the first band is enabled, the functionality is similar to the Singleband Sidechain Gate. Otherwise the signal is split in more bands and each band is processed individually. After the gating stage, the bands are mixed together to form the output result.

**Operating Mode**  
Determines how the input signal is split to obtain the different bands.

- **Classic** - The original signal is split using crossover filters. After the gating stage, all bands become phase-compensated using All-Pass filters.
- **Modern** - Each band is processed by a pair of dynamic shelving filters. This mode allows a better control over the gain of each band.

**Sidechain Boost**  
If enabled, it introduces a special mode for assigning the same weight for higher frequencies opposite to lower frequencies.

When disabled, the frequency band is processed 'as is', but the usual audio signal has 3 dB/octave falloff in the frequency domain and could be compared with the pink noise. So lower frequencies take more effect on the Gate rather than higher frequencies.

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

**External Sidechain**  
The Sidechain is an external source took by a specific input device (typically a microphone).

**Band Bypass**  
If enabled, the selected band is not affected by the gating stage.

**Solo**  
Turns on the Solo mode to the selected band by applying -36 dB gain to the other non-soloing bands.

**Mute**  
Turns on the Mute mode applying -36 dB gain to to the selected band.

**Attack Time**  
The length of time it takes to apply roughly two-thirds of the gain reduction.

**Release Time**  
The length of time it takes to restore roughly two-thirds of the gain reduction.

**Curve Threshold**  
The Gate fully opens upon the Sidechain level becoming above Curve Threshold.

If Hysteresis is not enabled, the Gate begins to close upon the Sidechain level becoming below Curve Threshold.

**Curve Zone Size**  
The Gate begins to open upon the Sidechain level becoming above the Curve Threshold + Curve Zone.

If Hysteresis is not enabled, the Gate fully closes upon the Sidechain level becoming below the Curve Threshold + Curve Zone.

**Hysteresis**  
When enabled, Curve Threshold and Curve Zone apply only to the opening Gate, and separate parameters can be configured for closing Gate.

**Hysteresis Threshold**  
If Hysteresis is enabled, the Gate begins to close upon the Sidechain level becoming below Curve Threshold + Hysteresis Threshold.

**Hysteresis Zone Size**  
If Hysteresis is enabled, the Gate fully closes upon the Sidechain level becoming below the Curve Threshold + Hysteresis Threshold + Hysteresis Zone.

**Reduction**  
If the value is negative, it acts as the amount of gain reduction to apply to the input signal when the Gate is fully closed. If the value is positive, the Gate operates in "Reverse Mode": It reduces the amplitude when the signal is above the threshold.

**Makeup**  
The gain to apply after the gating stage.

## Band Sidechain Options

**Mode**  
Determines how the Sidechain is evaluated for the gating stage.

- **Peak** - The Gate reacts according to the peaks.
- **RMS** - The Gate reacts according to the average loudness measured by the root mean square.
- **Low-Pass Filter** - The Gate reacts according to the signal processed by a recursive 1-pole Low-Pass filter.
- **Simple Moving Average** - The Gate reacts according to the signal processed by the Simple Moving Average filter.

**Source**  
Determines which part of the Sidechain is taken into account for the gating stage.

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
The time that defines the number of samples used to process the Sidechain in RMS, Uniform and Low-Pass modes. Higher the value, more smooth the gating.

**Lookahead**  
The band signal to gate is delayed by this amount of time, so that the gating will be applied earlier than it would be otherwise.

Each band can have different Lookahead values. To avoid phase distortions in the mixing stage, all the bands are automatically delayed for an individually calculated period of time.

## References

- [LSP Sidechain Multiband Gate Stereo](https://lsp-plug.in/?page=manuals&section=sc_mb_gate_stereo)
- [Wikipedia Noise Gate](https://en.wikipedia.org/wiki/Noise_gate)
