#!/usr/bin/env bash

# general settings

gsettings set com.github.wwmm.easyeffects enable-all-streaminputs false

# effects for sink inputs

gsettings set com.github.wwmm.easyeffects.autogain:/com/github/wwmm/easyeffects/streamoutputs/autogain/ state false
gsettings set com.github.wwmm.easyeffects.bassenhancer:/com/github/wwmm/easyeffects/streamoutputs/bassenhancer/ state false
gsettings set com.github.wwmm.easyeffects.compressor:/com/github/wwmm/easyeffects/streamoutputs/compressor/ state false
gsettings set com.github.wwmm.easyeffects.convolver:/com/github/wwmm/easyeffects/streamoutputs/convolver/ state false
gsettings set com.github.wwmm.easyeffects.crossfeed:/com/github/wwmm/easyeffects/streamoutputs/crossfeed/ state false
gsettings set com.github.wwmm.easyeffects.crystalizer:/com/github/wwmm/easyeffects/streamoutputs/crystalizer/ state false
gsettings set com.github.wwmm.easyeffects.deesser:/com/github/wwmm/easyeffects/streamoutputs/deesser/ state false
gsettings set com.github.wwmm.easyeffects.delay:/com/github/wwmm/easyeffects/streamoutputs/delay/ state false
gsettings set com.github.wwmm.easyeffects.equalizer:/com/github/wwmm/easyeffects/streamoutputs/equalizer/ state false
gsettings set com.github.wwmm.easyeffects.exciter:/com/github/wwmm/easyeffects/streamoutputs/exciter/ state false
gsettings set com.github.wwmm.easyeffects.filter:/com/github/wwmm/easyeffects/streamoutputs/filter/ state false
gsettings set com.github.wwmm.easyeffects.gate:/com/github/wwmm/easyeffects/streamoutputs/gate/ state false
gsettings set com.github.wwmm.easyeffects.limiter:/com/github/wwmm/easyeffects/streamoutputs/limiter/ state false
gsettings set com.github.wwmm.easyeffects.loudness:/com/github/wwmm/easyeffects/streamoutputs/loudness/ state false
gsettings set com.github.wwmm.easyeffects.maximizer:/com/github/wwmm/easyeffects/streamoutputs/maximizer/ state false
gsettings set com.github.wwmm.easyeffects.multibandcompressor:/com/github/wwmm/easyeffects/streamoutputs/multibandcompressor/ state false
gsettings set com.github.wwmm.easyeffects.multibandgate:/com/github/wwmm/easyeffects/streamoutputs/multibandgate/ state false
gsettings set com.github.wwmm.easyeffects.pitch:/com/github/wwmm/easyeffects/streamoutputs/pitch/ state false
gsettings set com.github.wwmm.easyeffects.reverb:/com/github/wwmm/easyeffects/streamoutputs/reverb/ state false
gsettings set com.github.wwmm.easyeffects.stereotools:/com/github/wwmm/easyeffects/streamoutputs/stereotools/ state false

# effects for source outputs

gsettings set com.github.wwmm.easyeffects.compressor:/com/github/wwmm/easyeffects/streaminputs/compressor/ state false
gsettings set com.github.wwmm.easyeffects.deesser:/com/github/wwmm/easyeffects/streaminputs/deesser/ state false
gsettings set com.github.wwmm.easyeffects.equalizer:/com/github/wwmm/easyeffects/streaminputs/equalizer/ state false
gsettings set com.github.wwmm.easyeffects.filter:/com/github/wwmm/easyeffects/streaminputs/filter/ state false
gsettings set com.github.wwmm.easyeffects.gate:/com/github/wwmm/easyeffects/streaminputs/gate/ state false
gsettings set com.github.wwmm.easyeffects.limiter:/com/github/wwmm/easyeffects/streaminputs/limiter/ state false
gsettings set com.github.wwmm.easyeffects.multibandcompressor:/com/github/wwmm/easyeffects/streaminputs/multibandcompressor/ state false
gsettings set com.github.wwmm.easyeffects.multibandgate:/com/github/wwmm/easyeffects/streaminputs/multibandgate/ state false
gsettings set com.github.wwmm.easyeffects.pitch:/com/github/wwmm/easyeffects/streaminputs/pitch/ state false
gsettings set com.github.wwmm.easyeffects.reverb:/com/github/wwmm/easyeffects/streaminputs/reverb/ state false
gsettings set com.github.wwmm.easyeffects.echocanceller:/com/github/wwmm/easyeffects/streaminputs/echocanceller/ state false
