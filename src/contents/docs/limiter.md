# Limiter

A Limiter is a special type of downward [Compressor](compressor.md) which does not allow the signal to overtake a predetermined Threshold. Ideally it has a very high compression ratio that takes the amplitude below a ceiling which stands as the maximum output level. For this reason it is usually named "brick-wall limiter".

Easy Effects uses the Sidechain Stereo Limiter from Linux Studio Plugins. In most cases it works as a brick-wall limiter, but it offers also an additional feature that acts like a Compressor with extreme settings, so the output signal may exceed the specified Threshold.

**Mode**  
Select the operative mode of the peak cutting algorithm which searches the peaks above the Threshold and applies short gain reduction patches to the signal.

These patches can be selected in 3 forms: **Hermite**, **Exponential** and **Linear**. Each one has 4 different variants related to gain reduction of the samples around the peak: **Thin**, **Tail**, **Duck** and **Wide**.

The shape of each form and variant can be referred into the Linux Studio Plugin manual. See References section at the bottom of the present document.

**Oversampling**  
When enabled, the sample rate is internally increased in order to improve peak detection and reduce aliasing (that causes distortion).

The modes have 2 main types: **Full**, which increases both the Sidechain and the Input signals, and **Half** which increases only the Sidechain. Each one of them specifies different **multipliers** and, between parentheses, the number of "**lobes** in the kernel".

The oversampled Input signal is downsampled to the original sample rate after processing.

**Dither**  
If specified, it enables Dithering for the selected bit depth. The process of Dithering adds a low-level noise to output signal in order to mask "quantization distortion", a form of artifact present in digital audio rendered at lower bit depth.

**SC PreAmp**  
The gain applied to the Sidechain before it is processed.

**Lookahead**  
The size of the buffer used to detect the peaks in advance. It adds the corresponding latency to the output signal.

**Attack**  
The length of time it takes to apply the needed gain reduction to keep the peak below the Threshold.

It affects the length of the gain reduction patch. It cannot be larger than the Lookahead (if specified larger, it's set internally as the max possible value).

**Release**  
The length of time it takes to restore the reduced gain around the limited peak.

It affects the length of the gain reduction patch. It cannot be twice larger than the Lookahead (if specified larger, it's set internally as the max possible value).

**Threshold**  
The target level above which the Limiter should reduce the peak of the signal. In some modes it represents the maximum output level.

**Threshold Boost**  
If enabled it applies an amount of gain to the limited signal equal to the absolute value of the Threshold. This causes the peak limited at the Threshold level to output at 0 dB (it has only effect when the Threshold is set below 0 dB).

**Stereo Link**  
The degree of the channel linking. At 0% both channels are limited independently while at 100% the loudest one triggers the same gain reduction on both.

**External Sidechain**  
Switch the Sidechain to an external source took by a specific input device (typically a microphone).

## Auto Leveling

The Auto Leveling checkbutton introduces an additional feature named "Automated Level Regulation" (ALR) which acts like a Compressor with infinite ratio for the purpose of applying a smoothed gain reduction rather than a stronger peak cutter like in brick-wall mode. This configuration could get the output level to exceed the Threshold even if the signal is highly compressed.

**Attack**  
Manage how the raise of the input signal affects the smoothness of the ALR curve that controls the gain reduction level. Higher the value, more quickly the curve goes to it's maximum.

**Release**  
Manage how the fall of the input signal affects the smoothness of the curve that controls the gain reduction level. Higher the value, more quickly the curve goes to it's minimum.

**Knee**  
Manage the Threshold of the ALR gain curve and, in fact, adjust the balance between two gain reduction stages. Raising the value delegates more work to the peak-cutting algorithm. Lowering the value delegates more work to the ALR gain reduction algorithm.

## References

- [Wikipedia Limiter](https://en.wikipedia.org/wiki/Limiter)
- [LSP Sidechain Limiter Stereo](https://lsp-plug.in/?page=manuals&section=sc_limiter_stereo)
