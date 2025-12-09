# Deep Noise Remover

Advanced noise reduction is achieved using "DeepFilterNet" technology. This technology uses AI technology called deep learning to efficiently remove noise from audio signals, enabling higher quality noise suppression than conventional methods. This technology has several control parameters, which are explained below and how to set them.

**Attenuation Limit**  
This parameter determines how much noise is attenuated. Higher values result in stronger noise reduction, but too high a value may also remove parts of the audio.

- Recommended setting: 70dB to 80dB is a typical balanced value. This minimizes background noise while preserving speech intelligibility.

**Minimum Processing Threshold**  
Signals below the Minimum Processing Threshold are targeted for denoising. Signals louder than this threshold are not subject to noise reduction and are passed through unchanged. This setting is intended to effectively remove small background noise.

- Recommended setting: -15dB to -30dB is good. This should be fine-tuned according to the ambient noise. Currently, Easy Effects does not allow a value smaller than -15dB.

**Maximum ERB Processing Threshold**  
The human auditory system has the ability to separate and recognize different frequency components. Equivalent Rectangular Bandwidth (ERB) is a numerical expression and model of this filtering capability. ERB-based processing emphasizes the perceptual importance of speech by focusing on specific frequency bands. This improves the effectiveness of noise reduction and speech enhancement. This threshold sets the maximum level at which perceptually equivalent bandwidth (ERB)-based processing is applied. Signals above this threshold are not subject to noise reduction or speech enhancement processing. This prevents important speech components or loud portions of speech from being unnecessarily processed.

- Recommended setting: Start in the range of 20dB to 30dB and adjust as needed.

**Maximum DF Processing Threshold**  
The Maximum DF Processing Threshold sets the maximum level of signal that DeepFilterNet will process. Signals exceeding this threshold are excluded from noise reduction processing. This prevents important or loud portions of speech from being unnecessarily processed.

- Recommended setting: 20 dB as the default value. This effectively suppresses noise while maintaining voice quality.

**Minimum Processing Buffer**  
Defines the size of the buffer used to process frames of audio data. A larger buffer can improve noise suppression but may increase latency.

- Recommended setting: Set to a small value (0) if real-time processing is important, or to a number greater than 1 if noise suppression accuracy is a priority.

**Post Filter Beta**  
Controls the intensity of the post-processing filter applied after the initial noise suppression. This allows for more subtle refinement of the audio signal.

- Recommended setting: 0.5dB to 2dB is generally recommended, but currently Easy Effects has a maximum value of 0.05dB, which is almost ineffective.
