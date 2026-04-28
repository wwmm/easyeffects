# Autotune

Autotune is a pitch correction effect that automatically adjusts the pitch of an audio signal to the nearest target note. It can be used to subtly correct vocal intonation or to create the characteristic "auto-tune effect" heard in modern music. Easy Effects uses the fat1 plugin from x42 LV2 Plugins.

## Tuning Options

**Mode**  
Controls how pitch correction is applied.

- **Auto** - Automatically detects the pitch and corrects it to the nearest enabled note in the scale.
- **Manual** - Targets a fixed pitch set by the Offset parameter.

**Tuning**  
The reference frequency for the A4 note, in Hz. The standard tuning is 440 Hz. Adjust this if you are working with non-standard tuning (range: 400–480 Hz).

**Correction**  
Controls the strength of pitch correction. At 1.0, the pitch is fully corrected to the target note. Lower values allow more of the natural pitch variation through, producing a subtler effect.

**Bias**  
Controls the preference toward the current note versus the nearest target note. At 0.5 (center), no bias is applied. Higher values make the correction stick to the current note longer before switching. Lower values make it switch to the nearest target more readily.

**Filter**  
Controls the smoothness of the pitch detection. Lower values result in faster, more responsive detection but may introduce artifacts. Higher values produce smoother, more stable detection but with slower response.

**Offset**  
Shifts the target pitch by the specified number of semitones (range: -2.0 to 2.0). In Manual mode, this sets the fixed target pitch offset.

**Fast Correction**  
When enabled, applies pitch correction with minimal latency. This produces a more immediate, aggressive correction effect but may introduce more artifacts.

## Scale Selection

The Scale section allows you to select which notes the pitch correction will target.

**Root**  
The root note of the scale (C, D, E, F, G, A, or B).

**Accidental**  
Applies a sharp or flat modifier to the root note.

**Scale**  
Selects either Major or Minor scale pattern. Choosing a scale automatically enables the correct notes for that key.

**Note Toggles**  
Individual toggles for each of the 12 chromatic notes (C through B). Selecting a scale sets these automatically, but you can also configure them manually for custom scales or chromatic correction (all notes enabled).

## Pitch Error Meter

Displays the current pitch deviation from the target note. A value of 0 means the pitch is exactly on target. Negative values indicate the pitch is flat, positive values indicate it is sharp.

## References

- [x42 fat1 Auto Tune](https://x42-plugins.com/x42/x42-autotune)
- [Wikipedia Auto-Tune](https://en.wikipedia.org/wiki/Auto-Tune)
