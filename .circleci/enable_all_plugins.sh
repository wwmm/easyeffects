#!/usr/bin/env bash

# general settings

gsettings set com.github.wwmm.easyeffects enable-all-streaminputs true

# effects for sink inputs

gsettings set com.github.wwmm.easyeffects.autogain:/com/github/wwmm/easyeffects/streamoutputs/autogain/ state true
gsettings set com.github.wwmm.easyeffects.bassenhancer:/com/github/wwmm/easyeffects/streamoutputs/bassenhancer/ state true
gsettings set com.github.wwmm.easyeffects.compressor:/com/github/wwmm/easyeffects/streamoutputs/compressor/ state true
gsettings set com.github.wwmm.easyeffects.convolver:/com/github/wwmm/easyeffects/streamoutputs/convolver/ state true
gsettings set com.github.wwmm.easyeffects.crossfeed:/com/github/wwmm/easyeffects/streamoutputs/crossfeed/ state true
gsettings set com.github.wwmm.easyeffects.crystalizer:/com/github/wwmm/easyeffects/streamoutputs/crystalizer/ state true
gsettings set com.github.wwmm.easyeffects.deesser:/com/github/wwmm/easyeffects/streamoutputs/deesser/ state true
gsettings set com.github.wwmm.easyeffects.delay:/com/github/wwmm/easyeffects/streamoutputs/delay/ state true
gsettings set com.github.wwmm.easyeffects.equalizer:/com/github/wwmm/easyeffects/streamoutputs/equalizer/ state true
gsettings set com.github.wwmm.easyeffects.exciter:/com/github/wwmm/easyeffects/streamoutputs/exciter/ state true
gsettings set com.github.wwmm.easyeffects.filter:/com/github/wwmm/easyeffects/streamoutputs/filter/ state true
gsettings set com.github.wwmm.easyeffects.gate:/com/github/wwmm/easyeffects/streamoutputs/gate/ state true
gsettings set com.github.wwmm.easyeffects.limiter:/com/github/wwmm/easyeffects/streamoutputs/limiter/ state true
gsettings set com.github.wwmm.easyeffects.loudness:/com/github/wwmm/easyeffects/streamoutputs/loudness/ state true
gsettings set com.github.wwmm.easyeffects.maximizer:/com/github/wwmm/easyeffects/streamoutputs/maximizer/ state true
gsettings set com.github.wwmm.easyeffects.multibandcompressor:/com/github/wwmm/easyeffects/streamoutputs/multibandcompressor/ state true
gsettings set com.github.wwmm.easyeffects.multibandgate:/com/github/wwmm/easyeffects/streamoutputs/multibandgate/ state true
gsettings set com.github.wwmm.easyeffects.pitch:/com/github/wwmm/easyeffects/streamoutputs/pitch/ state true
gsettings set com.github.wwmm.easyeffects.reverb:/com/github/wwmm/easyeffects/streamoutputs/reverb/ state true
gsettings set com.github.wwmm.easyeffects.stereotools:/com/github/wwmm/easyeffects/streamoutputs/stereotools/ state true

# effects for source outputs

gsettings set com.github.wwmm.easyeffects.compressor:/com/github/wwmm/easyeffects/streaminputs/compressor/ state true
gsettings set com.github.wwmm.easyeffects.deesser:/com/github/wwmm/easyeffects/streaminputs/deesser/ state true
gsettings set com.github.wwmm.easyeffects.equalizer:/com/github/wwmm/easyeffects/streaminputs/equalizer/ state true
gsettings set com.github.wwmm.easyeffects.filter:/com/github/wwmm/easyeffects/streaminputs/filter/ state true
gsettings set com.github.wwmm.easyeffects.gate:/com/github/wwmm/easyeffects/streaminputs/gate/ state true
gsettings set com.github.wwmm.easyeffects.limiter:/com/github/wwmm/easyeffects/streaminputs/limiter/ state true
gsettings set com.github.wwmm.easyeffects.multibandcompressor:/com/github/wwmm/easyeffects/streaminputs/multibandcompressor/ state true
gsettings set com.github.wwmm.easyeffects.multibandgate:/com/github/wwmm/easyeffects/streaminputs/multibandgate/ state true
gsettings set com.github.wwmm.easyeffects.pitch:/com/github/wwmm/easyeffects/streaminputs/pitch/ state true
gsettings set com.github.wwmm.easyeffects.reverb:/com/github/wwmm/easyeffects/streaminputs/reverb/ state true
gsettings set com.github.wwmm.easyeffects.webrtc:/com/github/wwmm/easyeffects/streaminputs/webrtc/ state true
