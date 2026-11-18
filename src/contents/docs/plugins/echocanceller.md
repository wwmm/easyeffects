# Echo Canceller

An echo is a reflected sound wave with sufficient magnitude and delay to be detectable as a signal distinct from the source one. The Echo Canceller is used to improve voice quality by preventing echo from being created or removing it after it has been added to the source signal. EasyEffects uses the Acoustic Echo Canceller (AEC3) from WebRTC AudioProcessing. Previous versions of EasyEffects used the SpeexDSP library.

## Options

Most of these options map directly to WebRTC's internal options.

### Echo Canceller

- **Enable** - Enable the echo canceller submodule.
- **Mobile mode** - Uses AECm, a lighter-weight alternative to AEC3 suitable for mobile devices with limited processing power, but typically produces worse results. This option may be removed in a future release, since [WebRTC has deprecated AECm](https://groups.google.com/g/discuss-webrtc/c/Vb__A68-3I0/m/RmMncfDeAQAJ).
- **Enforce high-pass** - This is effectively the same as "High-Pass Filter - Enable" option.

### Noise Suppression

- **Enable** - ???
- **Strength** (Low, Moderate, High, Very high) - ???

### High-Pass Filter

- **Enable** - ???
- **Full band** - ???

### Automatic gain control

???

## References

- [Wikipedia Echo Suppression and Cancellation](https://en.wikipedia.org/wiki/Echo_suppression_and_cancellation)
- No longer used: [Speex Acoustic Echo Canceller](https://www.speex.org/docs/manual/speex-manual/node4.html#SECTION00450000000000000000)
- [audio_processing.h · PulseAudio / webrtc-audio-processing · GitLab](https://gitlab.freedesktop.org/pulseaudio/webrtc-audio-processing/-/blob/master/webrtc/api/audio/audio_processing.h)
