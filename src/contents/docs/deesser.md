# Deesser

A Deesser is used to dynamically reduce high frequencies. The standard field of use of this plugin is the reduction of "sssss" and "shhhh" in vocal tracks. Easy Effects uses the Deesser developed by Calf Studio Gear.

**Detection**  
Select the detection of the Sidechain signal between Peak (stronger) and RMS (smoother).

**Mode**  
Select the operation mode between Wideband and Split. In Split mode not the full range signal will be affected by the gain reduction, but only frequencies above the split frequency will be manipulated in gain.

**F1 Split**  
The split frequency. All signals above this frequency will affect the gain reduction (and are affected in Split mode too).

**F1 Gain**  
It shifts the volume of the higher band. In Wideband mode it affects the Sidechain. In Split mode it also affects the processed high frequencies.

**F2 Peak**  
Center frequency of the bell filter. It allows a more precise selection of the Sidechain signal.

**F2 Level**  
Increases or decreases the level of the chosen F2 frequency.

**F2 Peak Q**  
Set the quality of the bell filter. Higher values will affect a narrower frequency range. Lower values will affect a wider band.

**Laxity**  
The reaction of the Deesser. Higher values won't affect really short peaks.

**Threshold**  
The level above which the gain reduction is applied.

**Ratio**  
The amount of attenuation applied to the signal.

For example, a Ratio of 2 means that if the level rises 4 dB above the Threshold, it will be only 2 dB above after the reduction.

**Makeup**  
The gain to apply after the processing stage. In Split mode only the high band will be made up.

## References

- [Wikipedia De-essing](https://en.wikipedia.org/wiki/De-essing)
- [Calf Deesser](https://calf-studio-gear.org/doc/Deesser.html)
- [LedgerNote - De-esser: The Guide for Sibilant-Free Vocal Recordings](https://ledgernote.com/columns/mixing-mastering/de-esser/)
