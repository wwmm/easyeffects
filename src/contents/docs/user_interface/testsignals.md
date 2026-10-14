# Test Signal

A specific signal can be used to test the frequency response of the speakers.

**Enable**  
Enable or disable the test signal.

**Channel**  
Choose if apply the test signal to left or right channel only, or both.

**Signal**  
The type of the signal: sine wave or white noise.

**Frequency**  
The frequency of the sine wave.

## Hearing Test

Open **Hearing Test** from an output Equalizer to create a listening profile for your headphones. Pause other audio, choose a comfortable system volume, and keep the headphones and volume unchanged throughout the test. Disable the regular test signal before starting.

For each ear, the test measures six frequencies from 250 Hz to 8 kHz. Each tone starts at a quiet level and plays for at most 1.5 seconds. Adjust **Tone Level**, play it again, and select **Record Threshold** when it is barely audible. Cancel if a tone is uncomfortable or cannot be heard within the available range. Changing the output device or its route cancels the test. The tones go directly to the selected output, bypassing the effects pipeline.

**Apply Personalized EQ** replaces the current Equalizer settings with separate bell filters for the left and right channels. Each band's gain is half the difference between its threshold and the lowest measured threshold across both ears, capped at 6 dB. Input gain is reduced by the larger sum of the channel gains to preserve headroom. These settings can be adjusted normally and saved in a preset for the tested headphones.

The thresholds are relative digital levels, not calibrated sound pressure or audiometric measurements. The result includes the response of the headphones and the listening environment. This is a subjective listening adjustment, not a diagnosis or a hearing aid prescription.
