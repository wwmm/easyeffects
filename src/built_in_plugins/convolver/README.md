# PulseEffects Convolver

Simple convolver plugin based on the library Zita-convolver and inspired in
the convolvers [https://github.com/tomszilagyi/ir.lv2/](https://github.com/tomszilagyi/ir.lv2/)
and [https://github.com/x42/convoLV2](https://github.com/x42/convoLV2).

You can test it from command line executing:

`gst-launch-1.0 -v audiotestsrc blocksize=512 ! peconvolver kernel-path=full_path_to_irs_file ! pulsesink`

This plugin only works with a power of two blocksize [64,128,256,512,1024,2048,4096]. 
