# Maximizer

The Maximizer is a special type of [Limiter](limiter.md) that does not only prevent the signal to exceed a specified target level, but also adjusts the average Loudness of the audio track.

Easy Effects uses the Maximizer developed by ZamAudio. It acts like an Amplifier that feeds a brick-wall Limiter with a fixed Lookahead of 10 ms adding the corresponding delay to the output signal.

**Threshold**  
This parameter represents the ideal amplification level needed by the signal.

It contributes along with the Ceiling to determine the amount of gain to apply to the signal before the limiting stage. The gain is calculated by `(-Threshold) - (-Ceiling)`.

For example, on Threshold -6 dB and Ceiling -2 dB, the signal is amplified by 4 dB and limited to not exceed -2 dB output level.

**Ceiling**  
This parameter represents the ideal attenuation level needed by the signal and the maximum allowed output level.

When the Threshold is set to 0 dB, the Ceiling is simply the gain reduction. When the Threshold is lowered, the signal is boosted without overtaking the Ceiling value.

**Release**  
Sets the release of the internal brick-wall Limiter. Lower values may introduce small artifacts.

## References

- [Sonarworks Blog - What Is A Maximizer?](https://www.sonarworks.com/soundid-reference/blog/learn/what-is-a-maximizer/)
