# Auto Gain

Easy Effects Autogain is based on the library libebur128 which implements the EBU R 128 standard for loudness normalization. It changes the audio volume to a perceived loudness target that can be customized by the user.

**Target**  
Loudness level.

**Silence**  
Silence threshold. While the momentary loudness is below this value the autogain won't make any changes to the current gain being applied to the signal level.

**Maximum History**  
Range of time taken into account for the calculation of loudness level and output gain.

**Reference**  
The parameter used as reference to evaluate the output gain.

- **Momentary** - Measures the loudness of the past 400 milliseconds.
- **Short-Term** - Measures the loudness of the past 3 seconds.
- **Integrated** - Ideally it indicates how loud the content is on average. It measures the loudness on a long range of time, depending on the value set as Maximum History.
- **Geometric Mean** - Uses the geometric mean of all the above mentioned parameters, or two of them at user choosing.

**Reset History**  
Resets the Autogain history related to chosen Reference.

**Monitor Parameters**  
Autogain values shown as stats.

- **Relative** - Used to detect silence. Whenever the Momentary term is below a predetermined threshold, modifications to the output gain will be disabled.
- **Loudness** - The difference between its value and the target loudness determines the output gain.
- **Range** - Indicates how large is the dynamic range of the content played.
- **Output Gain** - The input signal is adjusted by this correction gain to bring its loudness to the target value.

## References

- [Wikipedia EBU R 128](https://en.wikipedia.org/wiki/EBU_R_128)
- [EBU - Loudness Normalisation and Permitted Maximum Level of Audio Signals](https://tech.ebu.ch/publications/r128/)
