# Compressor

A Compressor is used to reduce the dynamic range or, in other words, the difference in level between the quietest and the loudest parts of an audio signal. It achieves this purpose altering the gain when the signal overtakes a predetermined Threshold. Easy Effects uses the Stereo Sidechain Compressor from Linux Studio Plugins.

## Compressor Options

**Attack Time**  
The length of time it takes to apply roughly two-thirds of the targeted amount of compression ratio to the uncompressed signal.

**Release Time**  
The length of time it takes to restore roughly two-thirds of the reduced gain (in Downward mode) or increased gain (in Upward/Boosting mode) to the compressed signal.

**Attack Threshold**  
The target level around which the compression is applied (the range depends by the Knee).

**Release Threshold**  
Sets up the Threshold of the Release Time, calculated by summing the Release Threshold to the Attack Threshold.

If the Sidechain level is above that Threshold, the compressor uses the Release Time for the releasing stage. Otherwise the Attack Time is used in place of Release Time.

For example, with -10 dB Attack Threshold and -60 dB Release Threshold, if the Sidechain is above `-10 + (-60) = -70 dB`, the Release Time is used for the gain restoration. If the Sidechain is below -70 dB, the Attack Time is used for the gain restoration.

**Ratio**  
The amount of attenuation (in Downward mode) or amplification (in Upward/Boosting mode) that will be applied to the signal.

For example, when the Ratio is 2 in Downward mode and the Sidechain rises above the Threshold by 10 dB, the signal would be ideally reduced by 5 dB (10/2 dB). In practice this behavior mostly depends on how the Compressor is designed and configured.

**Knee**  
The range over which the Compressor switches from no compression to almost the full ratio compression (the Threshold usually sits at the center of this transition zone).

**Makeup**  
The gain to apply after the compression stage.

**Dry Level**  
Amount of unprocessed signal mixed in the output.

**Wet Level**  
Amount of processed signal mixed in the output.

**Mode**  
- **Downward** - It's aimed to decrease the gain of the signal above the Threshold.
- **Upward** - It's aimed to increase the gain of the signal below the Threshold.
- **Boosting** - It's aimed to increase the gain of the signal below the Threshold by a specific amount.

**Boost Threshold**  
The Threshold below which a constant amplification will be applied to the input signal in Upward Mode (it prevents from applying infinite amplification to very quiet signals).

**Boost Amount**  
Maximum gain amplification to apply in Boosting Mode.

## Sidechain

**Listen**  
Allows to listen the processed Sidechain signal.

**Input Type**  
Determines which signal is the Sidechain or, in other words, the signal that controls the compression stage.

- **Feed-forward** - The Sidechain is the Compressor input signal (taken after applying the plugin input gain). More aggressive compression.
- **Feed-back** - The Sidechain is the Compressor output signal (taken before applying the Makeup and the plugin output gain). Vintage-style compression.
- **External** - The Sidechain is an external source took by a specific input device (typically a microphone).

**Input Device**  
Select the device for the External Sidechain.

**Mode**  
Determines how the Sidechain is evaluated for the compression stage.

- **Peak** - The Compressor reacts according to the peaks.
- **RMS** - The Compressor reacts according to the average loudness measured by the root mean square.
- **Low-Pass** - The Compressor reacts according to the signal processed by a Low-Pass filter.
- **Uniform** - The Compressor reacts according to the loudness measured by the average of the absolute amplitude.

**Source**  
Determines which part of the Sidechain is taken into account for the compression stage.

- **Middle** - The sum of left and right channels.
- **Side** - The difference between left and right channels.
- **Left** - Only left channel is used.
- **Right** - Only right channel is used.
- **Min** - The absolute minimum value is taken from stereo input.
- **Max** - The absolute maximum value is taken from stereo input.

**PreAmplification**  
Gain applied to the Sidechain signal.

**Reactivity**  
The time that defines the number of samples used to process the Sidechain in RMS, Uniform and Low-Pass modes. Higher the value, more smooth the compression.

**Lookahead**  
The signal to compress is delayed by this amount of time, so that the compression will be applied earlier than it would be otherwise. The corresponding delay is reproduced on the output signal.

## Sidechain Filters

**High-Pass Filter Mode**  
Sets the type of the High-Pass filter applied to Sidechain signal.

**High-Pass Frequency**  
Sets the cut-off frequency of the High-Pass filter.

**Low-Pass Filter Mode**  
Sets the type of the Low-Pass filter applied to Sidechain signal.

**Low-Pass Frequency**  
Sets the cut-off frequency of the Low-Pass filter.

## References

- [Wikipedia Dynamic Range Compression](https://en.wikipedia.org/wiki/Dynamic_range_compression)
- [LSP Sidechain Compressor Stereo](https://lsp-plug.in/?page=manuals&section=sc_compressor_stereo)
- [Black Ghost Audio - The Ultimate Guide to Compression](https://www.blackghostaudio.com/blog/the-ultimate-guide-to-compression)
- [Attack Magazine - Demolishing The Myths of Compression](https://www.attackmagazine.com/features/columns/gregory-scott-demolishing-the-myths-of-compression/)
