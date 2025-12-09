# Expander

An expander is an effect that works inversely to a compressor, further reducing the volume of quiet sounds below a certain level. This allows for natural suppression of background noise and unwanted whispers. For example, it is effective when air conditioning or other environmental sounds intrude into quiet scenes.

This effect is similar to the gate, which is explained in another chapter, in that it blocks quiet sounds. However, while a gate completely cuts sound, an expander gradually reduces sound, resulting in a more natural and smooth decay characteristic. Its main uses are listed below:

- Reduce the prominence of quiet environmental noises (air conditioning, PC fans, clicks, etc.) during periods of silence
- Suppress noise in quiet sections while maintaining the clarity of narration and dialogue
- Emphasize the contours (punch) of sounds to create a more dynamic impression

## Expander Options

The following is an explanation of each parameter when the mode is set to "**Downward**."

**Attack Time**: This determines the time it takes for the expander to begin suppressing the sound after the sound volume drops below the threshold. Shorter values will reduce the volume immediately, quickly suppressing noise and other imperfections. Longer values will cause the processing to begin later, preserving the natural reverberation and nuances of soft sounds.

**Release Time**: This determines the time it takes for the expander to stop suppressing the volume and return to its original volume after the sound exceeds the threshold again and is determined to no longer require processing.

Longer values will cause the sound to return more slowly, making it sound more natural.

Shorter values will return the sound to its original volume quickly, making it sound clearer and tighter, but may sound unnatural in some cases.

**Attack Threshold**: The effect is only applied to sounds quieter than this value.

**Release Threshold**: This can be the same as the Attack Threshold, but setting it about -5dB lower than the Attack Threshold will enable the Hysteresis function, suppressing fluttering around the threshold and achieving more stable processing.

**Ratio**: This is an indicator of how much quieter (= how much) sounds below the threshold will be reduced. Set the value using the following as a guide:

- A setting of 1 passes the signal as is (same as Expander OFF)
- For light noise suppression (natural processing), use 1.5-3
- For clarity of dialogue and smoothing out quiet sections, use 4-8
- For complete noise reduction (gate-like behavior), use 20-100

*Please note that setting the ratio too high can result in a choppy, unnatural sound.

**Knee**: This parameter adjusts how smoothly processing begins near the threshold. The name comes from the fact that the curve on the graph resembles a knee. See below for the relationship between the setting value and the curve.

Knee Value Volume Curve Characteristics Processing Sensation

0dB Abrupt bend (hard knee) The effect is applied suddenly (like an ON/OFF switch)

-6dB Gradual bend Processing begins naturally

-12dB and above Very smooth Processing is barely noticeable

**Makeup**: The gain applied after gating.

**Dry Level**: The amount of unprocessed signal mixed with the output.

**Wet Level**: The amount of processed signal mixed with the output.

For information on how to set up sidechaining and sidechain filtering, see the chapter of Gate.

*When "**Upward**" is selected in the mode setting, the expander will function to boost (increase the volume of) quieter sounds.

This can be used in situations where dialogue or narration is too quiet to be heard, and is also effective when you want to bring out the subtle nuances of a musical performance (such as pianissimo).

Rather than "lowering louder sounds" like a compressor, it is used to increase the average volume by "boosting quieter sounds," or to increase sound pressure while maintaining the punch and three-dimensionality of the original sound.