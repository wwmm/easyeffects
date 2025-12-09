# Loudness

Easy Effects uses the Loudness Compensator from Linux Studio Plugins which applies the "equal-loudness contour" corrections to the input signal.

An equal-loudness contour is a measure of sound pressure level (SPL), over the frequency spectrum, for which a listener perceives a constant loudness when presented with pure steady tones. The unit of measurement for loudness levels is the phon and is arrived at by reference to equal-loudness contours.

Usage of equal-loudness contours solves many mixing problems that every sound engineer meets while working on the track. The main problem is that human ear perceives different frequencies for different volume settings in a different way. In other words, applying changes to the mix on the low volume settings may cause unexpected sounding of the mix at the maximum loudness.

The Loudness Compensator performs frequency response computations and applies the computed frequency response to the input signal depending on the output volume settings. Additionally it can provide ear protection by applying hard-clipping to the output signal if it exceeds the allowed configurable level.

**Standard**  
Allows to select different equal-loudness contour standards.

- **Flat** - Applies flat frequency response to the whole spectrum. It's similar to just a gain knob but useful to perform a comparison to other modes.
- **ISO 226:2003** - Recent equal-loudness contour standard published in 2003.
- **Fletcher-Munson** - The first equal-loudness contour implementation by Harvey Fletcher and Wilden A. Munson published in 1933.
- **Robinson-Dadson** - More accurate equal-loudness contour implementation by D.W. Robinson and R.S. Dadson published in 1956. It became the basis for the ISO 226:2003 standard.

**FFT Size**  
Allows to select size of the Fast Fourier Transform frame used for the processing. The larger FFT frame is, the more precise the curve is approximated and the more latency is introduced.

**Output Volume**  
The output volume of the signal with applied equal loudness contour. It controls the loudness of the 1 kHz pure sine wave.

**Clipping**  
Allows to enable and the hard clipping of the output signal.

**Clipping Range**  
Allows to set the gap level for the hard clipping of the output signal.

## References

- [Wikipedia Equal-Loudness Contour](https://en.wikipedia.org/wiki/Equal-loudness_contour)
- [LSP Loudness Compensator Stereo](https://lsp-plug.in/?page=manuals&section=loud_comp_stereo)
- [Lindos Electronics - Equal-Loudness Contours](http://www.lindos.co.uk/cgi-bin/FlexiData.cgi?SOURCE=Articles&VIEW=full&id=17)
