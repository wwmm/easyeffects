# -*- coding: utf-8 -*-

from gi.repository import Gio, GLib


class WebrtcPresets():

    def __init__(self, config):
        self.config = config  # configParser

        self.settings_sourceoutputs = Gio.Settings(
            'com.github.wwmm.pulseeffects.sourceoutputs.webrtc')

    def add_section(self, settings, section):
        enabled = settings.get_value('state')

        high_pass = settings.get_value('high-pass-filter')
        extended_filter = settings.get_value('extended-filter')
        delay_agnostic = settings.get_value('delay-agnostic')

        target_level_dbfs = settings.get_value('target-level-dbfs')
        compression_gain_db = settings.get_value('compression-gain-db')
        limiter = settings.get_value('limiter')

        gain_control = settings.get_value('gain-control')
        gain_control_mode_adaptive = settings.get_value(
            'gain-control-mode-adaptive')
        gain_control_mode_fixed = settings.get_value(
            'gain-control-mode-fixed')

        echo_cancel = settings.get_value('echo-cancel')
        echo_suppression_level_low = settings.get_value(
            'echo-suppression-level-low')
        echo_suppression_level_moderate = settings.get_value(
            'echo-suppression-level-moderate')
        echo_suppression_level_high = settings.get_value(
            'echo-suppression-level-high')

        noise_suppression = settings.get_value('noise-suppression')
        noise_suppression_level_low = settings.get_value(
            'noise-suppression-level-low')
        noise_suppression_level_moderate = settings.get_value(
            'noise-suppression-level-moderate')
        noise_suppression_level_high = settings.get_value(
            'noise-suppression-level-high')
        noise_suppression_level_very_high = settings.get_value(
            'noise-suppression-level-very-high')

        voice_detection = settings.get_value('voice-detection')
        voice_detection_frame_size = settings.get_value(
            'voice-detection-frame-size-ms')
        voice_detection_likehood_very_low = settings.get_value(
            'voice-detection-likelihood-very-low')
        voice_detection_likehood_low = settings.get_value(
            'voice-detection-likelihood-low')
        voice_detection_likehood_moderate = settings.get_value(
            'voice-detection-likelihood-moderate')
        voice_detection_likehood_high = settings.get_value(
            'voice-detection-likelihood-high')

        self.config[section] = {'enabled': str(enabled),
                                'high_pass': str(high_pass),
                                'extended_filter': str(extended_filter),
                                'delay_agnostic': str(delay_agnostic),
                                'target_level_dbfs': str(target_level_dbfs),
                                'compression_gain_db':
                                str(compression_gain_db),
                                'limiter': str(limiter),
                                'gain_control': str(gain_control),
                                'gain_control_mode_adaptive':
                                str(gain_control_mode_adaptive),
                                'gain_control_mode_fixed':
                                str(gain_control_mode_fixed),
                                'echo_cancel': str(echo_cancel),
                                'echo_suppression_level_low':
                                str(echo_suppression_level_low),
                                'echo_suppression_level_moderate':
                                str(echo_suppression_level_moderate),
                                'echo_suppression_level_high':
                                str(echo_suppression_level_high),
                                'noise_suppression': str(noise_suppression),
                                'noise_suppression_level_low':
                                str(noise_suppression_level_low),
                                'noise_suppression_level_moderate':
                                str(noise_suppression_level_moderate),
                                'noise_suppression_level_high':
                                str(noise_suppression_level_high),
                                'noise_suppression_level_very_high':
                                str(noise_suppression_level_very_high),
                                'voice_detection': str(voice_detection),
                                'voice_detection_frame_size':
                                str(voice_detection_frame_size),
                                'voice_detection_likehood_very_low':
                                str(voice_detection_likehood_very_low),
                                'voice_detection_likehood_low':
                                str(voice_detection_likehood_low),
                                'voice_detection_likehood_moderate':
                                str(voice_detection_likehood_moderate),
                                'voice_detection_likehood_high':
                                str(voice_detection_likehood_high)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)

        settings.set_value('state', GLib.Variant('b', enabled))

    def save(self):
        self.add_section(self.settings_sourceoutputs, 'mic_webrtc')

    def load(self):
        self.load_section(self.settings_sourceoutputs, 'mic_webrtc')
