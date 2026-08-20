# Mid-Side Equalizer

The Mid-Side Equalizer processes the Mid (sum) and Side (difference) components of a stereo signal independently. This is useful to, for example, correct the overall tonality of a recording through the Mid channel while leaving only the stereo ambience untouched through the Side channel. Easy Effects uses the Parametric Equalizer x32 MidSide from Linux Studio Plugins. The user can choose from 1 to 32 bands per channel. Width and center frequency of each band can be customized as needed.

## Global Options

**Bands**  
The number of bands.

**Mode**  
- **IIR** - Infinite Impulse Response filters, nonlinear minimal phase. In most cases does not add noticeable latency to output signal.
- **FIR** - Finite Impulse Response filters with linear phase, finite approximation of Equalizer's impulse response. Adds noticeable latency to output signal.
- **FFT** - Fast Fourier Transform approximation of the frequency chart, linear phase. Adds noticeable latency to output signal.

**Balance**  
Balance between left and right output channels.

**Pitch Mid**  
The frequency shift for all filters of the mid channel, in semitones.

**Pitch Side**  
The frequency shift for all filters of the side channel, in semitones.

**Link Mid/Side**  
When enabled it is possible to apply different configurations to mid and side channels.

**Flat Response**  
This function sets each band gain to 0.

**Calculate Frequencies**  
This function calculates the center frequency and the width of each band using the current number of bands. Useful when the user wants fewer than 32 bands but has no idea about which frequencies should be chosen.

## Band Options

**Type**  
- **Off** - The filter is not working (turned off).
- **Bell** - Bell filter with smooth peak/recess.
- **High Pass** - High Pass filter with rejection of low frequencies.
- **High Shelf** - Shelving filter with adjustment of high frequency range.
- **Low Pass** - Low Pass filter with rejection of high frequencies.
- **Low Shelf** - Shelving filter with adjustment of low frequency range.
- **Notch** - Notch filter with full rejection of selected frequency.
- **Resonance** - Resonance filter with sharp peak/recess.
- **All Pass** - All Pass filter.

**Mode**  
- **RLC** - Very smooth filters based on similar cascades of RLC contours. Bilinear Z-transform (BT) or Matched Z-transform (MT) is used for pole/zero mapping.
- **BWC** - Butterworth-Chebyshev-type-1 based filters. Does not affect Resonance and Notch filters. Bilinear Z-transform (BT) or Matched Z-transform (MT) is used for pole/zero mapping.
- **LRX** - Linkwitz-Riley based filters. Does not affect Resonance and Notch filters. Bilinear Z-transform (BT) or Matched Z-transform (MT) is used for pole/zero mapping.
- **APO** - Digital biquad filters derived from canonic analog biquad prototypes digitalized through Bilinear transform. These are [textbook filters](https://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html) which are implemented as in the [Equalizer APO](https://equalizerapo.com/) software. Direct design (DR) is used to serve the digital filter coefficients directly in the digital domain, without performing transforms.

**Slope**  
The slope of the filter characteristics.

**Solo**  
Makes the selected band the only one active.

**Mute**  
Mutes the selected band.

**Frequency**  
Center frequency of the selected band.

**Width**  
Bandwidth calculated as `width = frequency / quality`.

**Quality**  
The quality factor of the filter used.

## References

- [Wikipedia Equalization (audio)](https://en.wikipedia.org/wiki/Equalization_(audio))
- [LSP Parametric Equalizer x32 MidSide](http://lsp-plug.in/?page=manuals&section=para_equalizer_x32_ms)
- [Wikipedia Q Factor](https://en.wikipedia.org/wiki/Q_factor)
- [How to EQ - Q Factor and Bandwidth in EQ: What They Mean](https://howtoeq.wordpress.com/2010/10/07/q-factor-and-bandwidth-in-eq-what-it-all-means/)
