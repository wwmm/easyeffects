# Voice Suppressor

Very simple plugin that removes voice from audio. It calculates the correlation of the lefft and right channels in
the frequency domain. Frequencies whose correlation and phase difference are beyond the configured thresholds are attenuated.  

**Start**  

Frequencies below this value are not attenuated.  

**End**  

Frequencies above this value value are not attenuated.  

**Correlation**  

Frequencies are attenuated only if the correlation between left and right channel is at least equal to this value.  

**Phase Difference**  

Frequencies are not attenuated when the magnitude of phase difference between left and right channel is above this value.  

**Minimum Kurtosis**  

Frequencies with local kurtosis below this value are attenuated.  

**Maximum Instantaneous Frequency**  

This is related to the frequency of rotation of the complex vector that represents the correlation between the left and right channel at a given frequency.
In other words this measures how fast the phase difference between the left and right channels changes at a given frequency bin in the Fourier transform.
More attenuation is applied to the channels samples that have rotation frequency below this parameter value.  

**Inverted Mode**  
Instead of suppressing voice the plugin will try to suppress the background and keep the voice.  
