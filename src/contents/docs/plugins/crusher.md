# Crusher

A bitcrusher reduces the resolution or bandwidth of digital audio data. Audio reduced in bit depth sounds more harsh and "digital". 
In the bitcrusher from Calf Studio Gear used in EasyEffects reduction can be done in a linear or logarithmic way.
According to the plugin authors the logarithmic way results in a much smoother sound in low volume signals. 

**Mode**  
- **Linear** - Linear distance between bits.
- **Logarithmic** - Logarithmic distances between bits. The result is a much more "natural" sounding crusher which doesn't gate low signals. 

**Bit reduction**  
Controls the processed audio bit depth. 

**DC offset**  
This offset causes different crushing of the lower and the upper half of the signal. 

**Anti-aliasing**  
Controls the softness of the crushing sounds. 

**Mix**  
Controls the mix of original and processed audio. When at 0% the output signal is equal to the original. When at 100% only the processed signal is in the output signal. 

**Reduction**  
Controls the reduction of the sample rate (downsampling).  

**Low frequency oscillator active**  
Enable the low frequency oscillator. The oscillator will make the sample rate reduction change between a range determined by the range control.

**Low frequency oscillator range**  
Controls the amount of modulation applied to the sample rate reduction. 

**Low frequency oscillator rate**  
Controls the frequency of the oscillator. 

## References

- [Wikipedia Bit Crusher](https://en.wikipedia.org/wiki/Bitcrusher)
- [Calf Crusher](https://calf-studio-gear.org)
- [Calf Crusher Documentation](https://calf-studio-gear.org/doc/Crusher.html)
