# Gate

The Gate attenuates signals that register below a Threshold. This kind of signal processing is used to reduce disturbing noise between useful signals. Easy Effects uses the Stereo Sidechain Gate from Linux Studio Plugins.

## Gate Workflow

The Gate begins to open when the Sidechain level becomes above the Attack Zone Start level.

The Gate fully opens when the Sidechain level becomes above the Attack Threshold level.

The Gate begins to close when the Sidechain level becomes below the Release Zone Start level.

The Gate fully closes when the Sidechain level becomes below the Release Threshold level.

## Gate Options

**Attack Time**  
The length of time it takes to restore roughly two-thirds of the gain reduction.

**Release Time**  
The length of time it takes to apply roughly two-thirds of the gain reduction.

**Curve Threshold**  
The Gate fully opens upon the Sidechain level becoming above Curve Threshold (displayed as Attack Threshold level).

If Hysteresis is not enabled, the Gate begins to close upon the Sidechain level becoming below Curve Threshold (displayed as Release Zone Start level).

**Curve Zone Size**  
The Gate begins to open upon the Sidechain level becoming above the Curve Threshold + Curve Zone (displayed as Attack Zone Start level).

If Hysteresis is not enabled, the Gate fully closes upon the Sidechain level becoming below the Curve Threshold + Curve Zone (displayed as Release Threshold level).

**Hysteresis**  
When enabled, Curve Threshold and Curve Zone apply only to the opening Gate, and separate parameters can be configured for closing Gate.

**Hysteresis Threshold**  
If Hysteresis is enabled, the Gate begins to close upon the Sidechain level becoming below Curve Threshold + Hysteresis Threshold (displayed as Release Zone Start level).

**Hysteresis Zone Size**  
If Hysteresis is enabled, the Gate fully closes upon the Sidechain level becoming below the Curve Threshold + Hysteresis Threshold + Hysteresis Zone (displayed as Release Threshold level).

**Dry Level**  
Amount of unprocessed signal mixed in the output.

**Wet Level**  
Amount of processed signal mixed in the output.

**Reduction**  
If the value is negative, it acts as the amount of gain reduction to apply to the input signal when the Gate is fully closed. If the value is positive, the Gate operates in "Reverse Mode": It reduces the amplitude when the signal is above the threshold.

**Makeup**  
The gain to apply after the gating stage.

## Sidechain

**Listen**  
Allows to listen the processed Sidechain signal.

**Input Type**  
Determines which signal is the Sidechain or, in other words, the signal that controls the gating stage.

- **Internal** - The Sidechain is the Gate input signal (taken after applying the plugin input gain).
- **External** - The Sidechain is an external source took by a specific input device (typically a microphone).

**Input Device**  
Select the device for the External Sidechain.

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

**PreAmplification**  
Gain applied to the Sidechain signal.

**Reactivity**  
The time that defines the number of samples used to process the Sidechain in RMS, Uniform and Low-Pass modes. Higher the value, more smooth the gating.

**Lookahead**  
The signal to gate is delayed by this amount of time, so that the gating will be applied earlier than it would be otherwise. The corresponding delay is reproduced on the output signal.

## Sidechain Filters

**High-Pass Filter Mode**  
Sets the type of the High-Pass filter applied to Sidechain signal.

**High-Pass Frequency**  
Sets the cut-off frequency of the High-Pass filter.

**Low-Pass Filter Mode**  
Sets the type of the Low-Pass filter applied to Sidechain signal.

**Low-Pass Frequency**  
Sets the cut-off frequency of the Low-Pass filter.

## References

- [Wikipedia Dynamic Range Compression](https://en.wikipedia.org/wiki/Dynamic_range_compression)
- [LSP Sidechain Gate Stereo](https://lsp-plug.in/?page=manuals&section=sc_gate_stereo)
- [Wikipedia Noise Gate](https://en.wikipedia.org/wiki/Noise_gate)
