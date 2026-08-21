# Mid-Side Equalizer

The Mid-Side Equalizer processes the Mid (sum) and Side (difference) parts of a stereo signal independently. It's useful, for example, to fix the overall tone of a recording through the Mid channel while keeping the stereo ambience untouched on the Side channel. Easy Effects uses the Parametric Equalizer x32 MidSide from Linux Studio Plugins. You can run anywhere from 1 to 32 bands per channel, and tweak the width and center frequency of each band.

## Global Options

**Bands**  
Number of bands.

**Mode**  
- **IIR** - Infinite Impulse Response filters, nonlinear minimal phase. Usually doesn't add noticeable latency to the output.
- **FIR** - Finite Impulse Response filters with linear phase, a finite approximation of the equalizer's impulse response. Adds noticeable latency to the output.
- **FFT** - Fast Fourier Transform approximation of the frequency chart, linear phase. Adds noticeable latency to the output.

**Balance**  
Balance between the left and right output channels.

**Pitch Mid**  
Frequency shift for all mid channel filters, in semitones.

**Pitch Side**  
Frequency shift for all side channel filters, in semitones.

**Link Mid/Side**  
When enabled, you can apply different configurations to the mid and side channels.

**Flat Response**  
Resets all band gains to 0.

**Calculate Frequencies**  
Figures out the center frequency and width for each band based on the current band count. Handy if you want fewer than 32 bands but aren't sure which frequencies to pick.

## Band Options

**Type**  
- **Off** - Filter is disabled.
- **Bell** - Bell filter with smooth peak/recess.
- **High Pass** - High Pass filter that removes low frequencies.
- **High Shelf** - Shelving filter that adjusts the high frequency range.
- **Low Pass** - Low Pass filter that removes high frequencies.
- **Low Shelf** - Shelving filter that adjusts the low frequency range.
- **Notch** - Notch filter that fully rejects the selected frequency.
- **Resonance** - Resonance filter with sharp peak/recess.
- **All Pass** - All Pass filter.

**Mode**  
- **RLC** - Very smooth filters built from cascaded RLC contours. Uses Bilinear Z-transform (BT) or Matched Z-transform (MT) for pole/zero mapping.
- **BWC** - Butterworth-Chebyshev-type-1 based filters. Doesn't affect Resonance and Notch filters. Uses Bilinear Z-transform (BT) or Matched Z-transform (MT) for pole/zero mapping.
- **LRX** - Linkwitz-Riley based filters. Doesn't affect Resonance and Notch filters. Uses Bilinear Z-transform (BT) or Matched Z-transform (MT) for pole/zero mapping.
- **APO** - Digital biquad filters derived from canonical analog biquad prototypes digitalized through Bilinear transform. These are [textbook filters](https://shepazu.github.io/Audio-EQ-Cookbook/audio-eq-cookbook.html) implemented the same way as in [Equalizer APO](https://equalizerapo.com/). Direct design (DR) generates the digital filter coefficients directly in the digital domain, without any transforms.

**Slope**  
Slope of the filter curve.

**Solo**  
Makes the selected band the only active one.

**Mute**  
Mutes the selected band.

**Frequency**  
Center frequency of the band.

**Width**  
Bandwidth, calculated as `width = frequency / quality`.

**Quality**  
Quality factor of the filter.

## References

- [Wikipedia Equalization (audio)](https://en.wikipedia.org/wiki/Equalization_(audio))
- [LSP Parametric Equalizer x32 MidSide](http://lsp-plug.in/?page=manuals&section=para_equalizer_x32_ms)
- [Wikipedia Q Factor](https://en.wikipedia.org/wiki/Q_factor)
- [How to EQ - Q Factor and Bandwidth in EQ: What They Mean](https://howtoeq.wordpress.com/2010/10/07/q-factor-and-bandwidth-in-eq-what-it-all-means/)
