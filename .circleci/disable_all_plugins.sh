#!/usr/bin/env bash

# general settings

gsettings set com.github.wwmm.pulseeffects enable-all-sourceoutputs false

# effects for sink inputs

gsettings set com.github.wwmm.pulseeffects.autogain:/com/github/wwmm/pulseeffects/sinkinputs/autogain/ state false
gsettings set com.github.wwmm.pulseeffects.bassenhancer:/com/github/wwmm/pulseeffects/sinkinputs/bassenhancer/ state false
gsettings set com.github.wwmm.pulseeffects.compressor:/com/github/wwmm/pulseeffects/sinkinputs/compressor/ state false
gsettings set com.github.wwmm.pulseeffects.convolver:/com/github/wwmm/pulseeffects/sinkinputs/convolver/ state false
gsettings set com.github.wwmm.pulseeffects.crossfeed:/com/github/wwmm/pulseeffects/sinkinputs/crossfeed/ state false
gsettings set com.github.wwmm.pulseeffects.crystalizer:/com/github/wwmm/pulseeffects/sinkinputs/crystalizer/ state false
gsettings set com.github.wwmm.pulseeffects.deesser:/com/github/wwmm/pulseeffects/sinkinputs/deesser/ state false
gsettings set com.github.wwmm.pulseeffects.delay:/com/github/wwmm/pulseeffects/sinkinputs/delay/ state false
gsettings set com.github.wwmm.pulseeffects.equalizer:/com/github/wwmm/pulseeffects/sinkinputs/equalizer/ state false
gsettings set com.github.wwmm.pulseeffects.exciter:/com/github/wwmm/pulseeffects/sinkinputs/exciter/ state false
gsettings set com.github.wwmm.pulseeffects.filter:/com/github/wwmm/pulseeffects/sinkinputs/filter/ state false
gsettings set com.github.wwmm.pulseeffects.gate:/com/github/wwmm/pulseeffects/sinkinputs/gate/ state false
gsettings set com.github.wwmm.pulseeffects.limiter:/com/github/wwmm/pulseeffects/sinkinputs/limiter/ state false
gsettings set com.github.wwmm.pulseeffects.loudness:/com/github/wwmm/pulseeffects/sinkinputs/loudness/ state false
gsettings set com.github.wwmm.pulseeffects.maximizer:/com/github/wwmm/pulseeffects/sinkinputs/maximizer/ state false
gsettings set com.github.wwmm.pulseeffects.multibandcompressor:/com/github/wwmm/pulseeffects/sinkinputs/multibandcompressor/ state false
gsettings set com.github.wwmm.pulseeffects.multibandgate:/com/github/wwmm/pulseeffects/sinkinputs/multibandgate/ state false
gsettings set com.github.wwmm.pulseeffects.pitch:/com/github/wwmm/pulseeffects/sinkinputs/pitch/ state false
gsettings set com.github.wwmm.pulseeffects.reverb:/com/github/wwmm/pulseeffects/sinkinputs/reverb/ state false
gsettings set com.github.wwmm.pulseeffects.stereotools:/com/github/wwmm/pulseeffects/sinkinputs/stereotools/ state false

# effects for source outputs

gsettings set com.github.wwmm.pulseeffects.compressor:/com/github/wwmm/pulseeffects/sourceoutputs/compressor/ state false
gsettings set com.github.wwmm.pulseeffects.deesser:/com/github/wwmm/pulseeffects/sourceoutputs/deesser/ state false
gsettings set com.github.wwmm.pulseeffects.equalizer:/com/github/wwmm/pulseeffects/sourceoutputs/equalizer/ state false
gsettings set com.github.wwmm.pulseeffects.filter:/com/github/wwmm/pulseeffects/sourceoutputs/filter/ state false
gsettings set com.github.wwmm.pulseeffects.gate:/com/github/wwmm/pulseeffects/sourceoutputs/gate/ state false
gsettings set com.github.wwmm.pulseeffects.limiter:/com/github/wwmm/pulseeffects/sourceoutputs/limiter/ state false
gsettings set com.github.wwmm.pulseeffects.multibandcompressor:/com/github/wwmm/pulseeffects/sourceoutputs/multibandcompressor/ state false
gsettings set com.github.wwmm.pulseeffects.multibandgate:/com/github/wwmm/pulseeffects/sourceoutputs/multibandgate/ state false
gsettings set com.github.wwmm.pulseeffects.pitch:/com/github/wwmm/pulseeffects/sourceoutputs/pitch/ state false
gsettings set com.github.wwmm.pulseeffects.reverb:/com/github/wwmm/pulseeffects/sourceoutputs/reverb/ state false
gsettings set com.github.wwmm.pulseeffects.webrtc:/com/github/wwmm/pulseeffects/sourceoutputs/webrtc/ state false
