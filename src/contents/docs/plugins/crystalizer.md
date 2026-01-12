# Crystalizer

The Crystalizer plugin can be used to add a little of dynamic range to songs that were overly compressed. The signal is split in multiple bands to which different intensities can be applied in order to alter the overall dynamic range.

**Intensities**  
The higher the value the higher is the difference in magnitude between the loudest and the quietest sounds of the selected band. Different intensities can be set for each frequency band.

**Bypass**  
When active the audio signal passing through the selected band is not modified.

**Mute**  
Mutes the selected band.

**Adaptive Intensity**  
When enabled the intensity value set by the band slider is continously scaled as audio is processed. The scaling may
increase or decrease the intensity based on the geometric mean of the signal crest factor, kurtosis and spectral flux.

When using static intensity values it may happen that the crystalizer enhances high frequency audio too much. What leads
to audible artifacts. The adaptive scaling removes this is most of the cases.  

**Oversampling**
Resamples the signal to double its sampling rate value and back to the orignal value after the plugins effects are applied.
It improves the effects quality but it also increases CPU usage.  

**Oversampling Quality**
Controls the resampling quality. Higher values require more CPU power.  

**Fixed Quantum**
Forces the plugin to use a fixed audio buffer size to process audio. This helps to avoid noises when the sound server switches latency on the fly.
But it has the downside of introducing unneded latency in some cases. And increased CPU usage in cases where higher latency could be used.
## References

- [Wikipedia Dynamic Range](https://en.wikipedia.org/wiki/Dynamic_range)  
- [Crest factor](https://en.wikipedia.org/wiki/Crest_factor)  
- [Kurtosis](https://en.wikipedia.org/wiki/Kurtosis)  
- [Spectral flux](https://en.wikipedia.org/wiki/Spectral_flux)
