# -*- coding: utf-8 -*-

import gi
gi.require_version('Gst', '1.0')
import numpy as np
from gi.repository import Gio
from scipy.interpolate import CubicSpline
from PulseEffects.source_output_pipeline import SourceOutputPipeline
from PulseEffects.effects_ui_base import EffectsUiBase


class SourceOutputEffects(EffectsUiBase, SourceOutputPipeline):

    def __init__(self, sampling_rate):
        self.old_limiter_attenuation = 0
        self.old_compressor_gain_reduction = 0

        self.settings = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs')

        SourceOutputPipeline.__init__(self, sampling_rate)
        EffectsUiBase.__init__(self, '/ui/source_outputs_plugins.glade',
                               self.settings)

        self.builder.connect_signals(self)

    def on_message_element(self, bus, msg):
        plugin = msg.src.get_name()

        if plugin == 'limiter_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_limiter_input_level_left.set_value(l_value)
                self.ui_limiter_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_limiter_input_level_left.set_value(0)
                self.ui_limiter_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_limiter_input_level_right.set_value(r_value)
                self.ui_limiter_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_limiter_input_level_right.set_value(0)
                self.ui_limiter_input_level_right_label.set_text('-99')
        elif plugin == 'limiter_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_limiter_output_level_left.set_value(l_value)
                self.ui_limiter_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_limiter_output_level_left.set_value(0)
                self.ui_limiter_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_limiter_output_level_right.set_value(r_value)
                self.ui_limiter_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_limiter_output_level_right.set_value(0)
                self.ui_limiter_output_level_right_label.set_text('-99')

            attenuation = round(self.limiter.get_property('attenuation'))

            if attenuation != self.old_limiter_attenuation:
                self.old_limiter_attenuation = attenuation

                self.ui_limiter_attenuation_levelbar.set_value(attenuation)
                self.ui_limiter_attenuation_level_label.set_text(
                    str(round(attenuation)))
        elif plugin == 'compressor_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_compressor_input_level_left.set_value(l_value)
                self.ui_compressor_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_compressor_input_level_left.set_value(0)
                self.ui_compressor_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_compressor_input_level_right.set_value(r_value)
                self.ui_compressor_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_compressor_input_level_right.set_value(0)
                self.ui_compressor_input_level_right_label.set_text('-99')
        elif plugin == 'compressor_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_compressor_output_level_left.set_value(l_value)
                self.ui_compressor_output_level_left_label.set_text(
                    str(round(left)))

                # reverb input
                self.ui_reverb_input_level_left.set_value(l_value)
                self.ui_reverb_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_compressor_output_level_left.set_value(0)
                self.ui_compressor_output_level_left_label.set_text('-99')

                # reverb input
                self.ui_reverb_input_level_left.set_value(0)
                self.ui_reverb_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_compressor_output_level_right.set_value(r_value)
                self.ui_compressor_output_level_right_label.set_text(
                    str(round(right)))

                # reverb input
                self.ui_reverb_input_level_right.set_value(r_value)
                self.ui_reverb_input_level_right_label.set_text(
                    str(round(left)))
            else:
                self.ui_compressor_output_level_right.set_value(0)
                self.ui_compressor_output_level_right_label.set_text('-99')

                # reverb input
                self.ui_reverb_input_level_right.set_value(0)
                self.ui_reverb_input_level_right_label.set_text('-99')

            gain_reduction = abs(round(
                self.compressor.get_property('gain-reduction')))

            if gain_reduction != self.old_compressor_gain_reduction:
                self.old_compressor_gain_reduction = gain_reduction

                self.ui_compressor_gain_reduction_levelbar.set_value(
                    gain_reduction)
                self.ui_compressor_gain_reduction_level_label.set_text(
                    str(round(gain_reduction)))
        elif plugin == 'reverb_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_reverb_input_level_left.set_value(l_value)
                self.ui_reverb_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_reverb_input_level_left.set_value(0)
                self.ui_reverb_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_reverb_input_level_right.set_value(r_value)
                self.ui_reverb_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_reverb_input_level_right.set_value(0)
                self.ui_reverb_input_level_right_label.set_text('-99')
        elif plugin == 'reverb_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_reverb_output_level_left.set_value(l_value)
                self.ui_reverb_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_reverb_output_level_left.set_value(0)
                self.ui_reverb_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_reverb_output_level_right.set_value(r_value)
                self.ui_reverb_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_reverb_output_level_right.set_value(0)
                self.ui_reverb_output_level_right_label.set_text('-99')
        elif plugin == 'highpass_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_highpass_input_level_left.set_value(l_value)
                self.ui_highpass_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_highpass_input_level_left.set_value(0)
                self.ui_highpass_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_highpass_input_level_right.set_value(r_value)
                self.ui_highpass_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_highpass_input_level_right.set_value(0)
                self.ui_highpass_input_level_right_label.set_text('-99')
        elif plugin == 'highpass_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_highpass_output_level_left.set_value(l_value)
                self.ui_highpass_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_highpass_output_level_left.set_value(0)
                self.ui_highpass_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_highpass_output_level_right.set_value(r_value)
                self.ui_highpass_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_highpass_output_level_right.set_value(0)
                self.ui_highpass_output_level_right_label.set_text('-99')
        elif plugin == 'lowpass_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_lowpass_input_level_left.set_value(l_value)
                self.ui_lowpass_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_lowpass_input_level_left.set_value(0)
                self.ui_lowpass_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_lowpass_input_level_right.set_value(r_value)
                self.ui_lowpass_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_lowpass_input_level_right.set_value(0)
                self.ui_lowpass_input_level_right_label.set_text('-99')
        elif plugin == 'lowpass_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_lowpass_output_level_left.set_value(l_value)
                self.ui_lowpass_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_lowpass_output_level_left.set_value(0)
                self.ui_lowpass_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_lowpass_output_level_right.set_value(r_value)
                self.ui_lowpass_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_lowpass_output_level_right.set_value(0)
                self.ui_lowpass_output_level_right_label.set_text('-99')
        elif plugin == 'equalizer_input_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_equalizer_input_level_left.set_value(l_value)
                self.ui_equalizer_input_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_equalizer_input_level_left.set_value(0)
                self.ui_equalizer_input_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_equalizer_input_level_right.set_value(r_value)
                self.ui_equalizer_input_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_equalizer_input_level_right.set_value(0)
                self.ui_equalizer_input_level_right_label.set_text('-99')
        elif plugin == 'equalizer_output_level':
            peak = msg.get_structure().get_value('peak')

            left, right = peak[0], peak[1]

            if left >= -99:
                l_value = 10**(left / 20)
                self.ui_equalizer_output_level_left.set_value(l_value)
                self.ui_equalizer_output_level_left_label.set_text(
                    str(round(left)))
            else:
                self.ui_equalizer_output_level_left.set_value(0)
                self.ui_equalizer_output_level_left_label.set_text('-99')

            if right >= -99:
                r_value = 10**(right / 20)
                self.ui_equalizer_output_level_right.set_value(r_value)
                self.ui_equalizer_output_level_right_label.set_text(
                    str(round(right)))
            else:
                self.ui_equalizer_output_level_right.set_value(0)
                self.ui_equalizer_output_level_right_label.set_text('-99')
        elif plugin == 'spectrum':
            magnitudes = msg.get_structure().get_value('magnitude')

            cs = CubicSpline(self.spectrum_freqs,
                             magnitudes[:self.spectrum_nfreqs])

            magnitudes = cs(self.spectrum_x_axis)

            max_mag = np.amax(magnitudes)
            min_mag = self.spectrum_threshold

            if max_mag > min_mag:
                magnitudes = (min_mag - magnitudes) / min_mag

                self.emit('new_spectrum', magnitudes)

        return True

    def init_ui(self):
        self.init_limiter_ui()
        self.init_compressor_ui()
        self.init_reverb_ui()
        self.init_highpass_ui()
        self.init_lowpass_ui()
        self.init_equalizer_ui()

    def on_eq_flat_response_button_clicked(self, obj):
        self.apply_eq_preset([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    def on_eq_reset_freqs_button_clicked(self, obj):
        self.settings.reset('equalizer-freqs')
        self.init_eq_freq_and_qfactors()

    def on_eq_reset_qfactors_button_clicked(self, obj):
        self.settings.reset('equalizer-qfactors')
        self.init_eq_freq_and_qfactors()

    def reset(self):
        self.settings.reset('limiter-user')
        self.settings.reset('compressor-user')
        self.settings.reset('reverb-user')
        self.settings.reset('highpass-cutoff')
        self.settings.reset('highpass-poles')
        self.settings.reset('lowpass-cutoff')
        self.settings.reset('lowpass-poles')
        self.settings.reset('equalizer-input-gain')
        self.settings.reset('equalizer-output-gain')
        self.settings.reset('equalizer-user')
        self.settings.reset('equalizer-freqs')
        self.settings.reset('equalizer-qfactors')

        self.init_ui()
