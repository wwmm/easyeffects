#!/usr/bin/env bash

# general settings

gsettings set com.github.wwmm.pulseeffects enable-all-sourceoutputs true

# effects for sink inputs

gsettings set com.github.wwmm.pulseeffects.sinkinputs.autogain state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.bassenhancer state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.compressor state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.convolver state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.crossfeed state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.crystalizer state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.deesser state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.delay state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.equalizer state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.exciter state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.filter state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.gate state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.limiter state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.loudness state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.maximizer state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.multibandcompressor state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.multibandgate state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.pitch state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.reverb state true
gsettings set com.github.wwmm.pulseeffects.sinkinputs.stereotools state true

# effects for source outputs

gsettings set com.github.wwmm.pulseeffects.sourceoutputs.compressor state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.deesser state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.equalizer state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.filter state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.gate state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.limiter state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.multibandcompressor state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.multibandgate state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.pitch state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.reverb state true
gsettings set com.github.wwmm.pulseeffects.sourceoutputs.webrtc state true
