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

**Attack and Release Time**  

They control the smoothing of the attenuation factor applied to the signal that fits the criteria set by the parameters above.
A smaller attack time makes the attenation to be increased faster. Londer release times makes the current suppresion to stay for longer.