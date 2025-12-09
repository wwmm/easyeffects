# Echo Canceller

The Echo is a reflected sound wave with sufficient magnitude and delay to be detectable as a signal distinct from the source one. An Echo Canceller is used to improve voice quality by preventing Echo from being created or removing it after it has been added to the source signal. Easy Effects uses the Echo Canceller from SpeexDSP library.

**Frame Size**  
The amount of time in milliseconds to process at once. It is recommended to use a frame size in the order of 20 ms.

**Filter Length**  
The amount of time of the Echo cancelling filter to use (also known as tail length). The recommended tail length is approximately the third of the room reverberation time. For example, in a small room, reverberation time is in the order of 300 ms, so a tail length of 100 ms is a good choice.

## References

- [Wikipedia Echo Suppression and Cancellation](https://en.wikipedia.org/wiki/Echo_suppression_and_cancellation)
- [Speex Acoustic Echo Canceller](https://www.speex.org/docs/manual/speex-manual/node4.html#SECTION00450000000000000000)
