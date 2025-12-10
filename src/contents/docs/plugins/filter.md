# Filter

A Filter is used to amplify (boost), pass or attenuate (cut) defined parts of a frequency spectrum. Easy Effects uses the Filter from Linux Studio Plugins.

**Type**  
- **Low-pass** - Low-pass filter with rejection of high frequencies.  
- **High-pass** - High-pass filter with rejection of low frequencies.  
- **Low-shelf** - Shelving filter with adjustment of low frequencies.  
- **High-shelf** - Shelving filter with adjustment of high frequency range.  
- **Bell** - Bell filter with smooth peak/recess.  
- **Bandpass** - Bandpass filter.  
- **Notch** - Notch filter with full rejection of selected frequency.  
- **Resonance** - Resonance filter with sharp peak/recess.  
- **Ladder-pass** - The filter that makes some ladder-passing in the spectrum domain.  
- **Ladder-rej** - The filter that makes some ladder-rejection in the spectrum domain.  
- **Allpass** - All-pass filter which only affects the phase of the audio signal at the specified frequency.  

**Filter Mode**  
- **RLC** - Very smooth filters based on similar cascades of RLC contours.  
- **BWC** - Butterworth-Chebyshev-type-1 based filters. Does not affect Resonance and Notch filters.  
- **LRX** - Linkwitz-Riley based filters. Does not affect Resonance and Notch filters.  
- **APO** - Digital biquad filters derived from canonic analog biquad prototypes digitalized through Bilinear transform. These are textbook filters which are implemented as in the EqualizerAPO software.  
- **BT** - Bilinear Z-transform is used for pole/zero mapping.  
- **MT** - Matched Z-transform is used for pole/zero mapping.  
- **DR** - Direct design is used to serve the digital filter coefficients directly in the digital domain, without performing transforms.  

**Equalizer Mode**  
- **IIR** - Infinite Impulse Response filters, nonlinear minimal phase. In most cases does not add noticeable latency to output signal.  
- **FIR** - Finite Impulse Response filters with linear phase, finite approximation of equalizer's impulse response. Adds noticeable latency to output signal.  
- **FFT** - Fast Fourier Transform approximation of the frequency chart, linear phase. Adds noticeable latency to output signal.  
- **SPM** - Spectral Processor Mode of equalizer, equalizer transforms the magnitude of signal spectrum instead of applying impulse response to the signal.

**Slope**  
The slope of the filter characteristics.  

**Frequency**  
The cutoff/resonance frequency of the filter or the middle frequency of the band.  

**Width**  
The width of the bandpass/ladder filters in octaves.  

**Gain**  
The gain of the filter. It is disabled for lo-pass/hi-pass/notch filters.  

**Quality**  
The quality factor of the filter.  

**Balance**  
The balance between left and right output channels.  

## References

- [Wikipedia Audio Filter](https://en.wikipedia.org/wiki/Audio_filter)
- [Linux Studio Plugins Filter](https://lsp-plug.in/?page=manuals&section=filter_stereo)
