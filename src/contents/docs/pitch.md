# Pitch

Pitch shifting is a sound recording technique in which the original Pitch of a sound is raised or lowered. Easy Effects uses the pitch shifter from SoundTouch.

**Mode**  
Controls the method used for pitch shifting.

- **High Speed** - Uses a method with a CPU cost that is relatively moderate and predictable.
- **High Quality** - Uses the highest quality method for pitch shifting. This CPU cost is approximately proportional to the required frequency shift.
- **High Consistency** - Uses the method that gives greatest consistency when used to create small variations in pitch around the 1.0-ratio level. Unlike the previous two options, this avoids discontinuities when moving across the 1.0 pitch scale. It also consumes more CPU than the others in the case where the pitch scale is exactly 1.0.

**Formant**  
Controls the handling of formant shape (spectral envelope) when pitch-shifting.

- **Shifted** - Applies no special formant processing. The spectral envelope will be pitch shifted as normal.
- **Preserved** - Preserves the spectral envelope of the unshifted signal. This permits shifting the note frequency without so substantially affecting the perceived pitch profile of the voice or instrument.

**Transients**  
Controls the component frequency phase-reset mechanism that may be used at transient points to provide clarity and realism to percussion and other significant transient sounds.

- **Crisp** - Resets component phases at the peak of each transient (the start of a significant note or percussive event). This usually results in a clear-sounding output, but it is not always consistent, and may cause interruptions in stable sounds present at the same time as transient events.
- **Mixed** - Resets component phases at the peak of each transient, outside a frequency range typical of musical fundamental frequencies. The results may be more regular for mixed stable and percussive notes than Crisp option, but with a "phasier" sound. The balance may sound very good for certain types of music and fairly bad for others.
- **Smooth** - Does not reset component phases at any point. The results will be smoother and more regular but may be less clear than the other transient options.

**Detector**  
Controls the type of transient detector used.

- **Compound** - Uses a general purpose transient detector which is likely to be good for most situations.
- **Percussive** - Detects percussive transients.
- **Soft** - Uses an onset detector with less of a bias toward percussive transients. This may give better results with certain material (e.g. relatively monophonic piano music).

**Phase**  
Controls the adjustment of component frequency phases from one analysis window to the next during non-transient segments.

- **Laminar** - Adjusts phases when stretching in such a way as to try to retain the continuity of phase relationships between adjacent frequency bins whose phases are behaving in similar ways. This should give good results in most situations.
- **Independent** - Adjusts the phase in each frequency bin independently from its neighbours. This usually results in a slightly softer, phasier sound.

**Cents**  
Number of cents the Pitch will be increased or decreased.

**Semitones**  
Number of semitones the Pitch will be increased or decreased.

**Octaves**  
Number of octaves the Pitch will be increased or decreased.

## References

- [Wikipedia Pitch Shift](https://en.wikipedia.org/wiki/Pitch_shift)
- [SoundTouch Audio Time Stretcher Library - Attractive Features](https://www.surina.net/soundtouch/)
