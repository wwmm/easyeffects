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
        voice_detection_likelihood_very_low = settings.get_value(
            'voice-detection-likelihood-very-low')
        voice_detection_likelihood_low = settings.get_value(
            'voice-detection-likelihood-low')
        voice_detection_likelihood_moderate = settings.get_value(
            'voice-detection-likelihood-moderate')
        voice_detection_likelihood_high = settings.get_value(
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
                                'voice_detection_likelihood_very_low':
                                str(voice_detection_likelihood_very_low),
                                'voice_detection_likelihood_low':
                                str(voice_detection_likelihood_low),
                                'voice_detection_likelihood_moderate':
                                str(voice_detection_likelihood_moderate),
                                'voice_detection_likelihood_high':
                                str(voice_detection_likelihood_high)}

    def load_section(self, settings, section):
        enabled = self.config.getboolean(section, 'enabled', fallback=False)

        high_pass = self.config.getboolean(section, 'high_pass',
                                           fallback=True)
        extended_filter = self.config.getboolean(section, 'extended_filter',
                                                 fallback=True)
        delay_agnostic = self.config.getboolean(section, 'delay_agnostic',
                                                fallback=False)

        target_level_dbfs = self.config.getint(section, 'target_level_dbfs',
                                               fallback=3)
        compression_gain_db = self.config.getint(section,
                                                 'compression_gain_db',
                                                 fallback=9)
        limiter = self.config.getboolean(section, 'limiter', fallback=True)

        gain_control = self.config.getboolean(section, 'gain_control',
                                              fallback=True)
        gain_control_mode_adaptive = self.config.getboolean(
            section, 'gain_control_mode_adaptive', fallback=True)
        gain_control_mode_fixed = self.config.getboolean(
            section, 'gain_control_mode_fixed', fallback=False)

        echo_cancel = self.config.getboolean(section, 'echo_cancel',
                                             fallback=True)
        echo_suppression_level_low = self.config.getboolean(
            section, 'echo_suppression_level_low', fallback=False)
        echo_suppression_level_moderate = self.config.getboolean(
            section, 'echo_suppression_level_moderate', fallback=True)
        echo_suppression_level_high = self.config.getboolean(
            section, 'echo_suppression_level_high', fallback=False)

        noise_suppression = self.config.getboolean(section,
                                                   'noise_suppression',
                                                   fallback=True)
        noise_suppression_level_low = self.config.getboolean(
            section, 'noise_suppression_level_low', fallback=False)
        noise_suppression_level_moderate = self.config.getboolean(
            section, 'noise_suppression_level_moderate', fallback=True)
        noise_suppression_level_high = self.config.getboolean(
            section, 'noise_suppression_level_high', fallback=False)
        noise_suppression_level_very_high = self.config.getboolean(
            section, 'noise_suppression_level_very_high', fallback=False)

        voice_detection = self.config.getboolean(section, 'voice_detection',
                                                 fallback=True)
        voice_detection_frame_size = self.config.getint(
            section, 'voice_detection_frame_size', fallback=10)
        voice_detection_likelihood_very_low = self.config.getboolean(
            section, 'voice_detection_likelihood_very_low', fallback=False)
        voice_detection_likelihood_low = self.config.getboolean(
            section, 'voice_detection_likelihood_low', fallback=True)
        voice_detection_likelihood_moderate = self.config.getboolean(
            section, 'voice_detection_likelihood_moderate', fallback=False)
        voice_detection_likelihood_high = self.config.getboolean(
            section, 'voice_detection_likelihood_high', fallback=False)

        settings.set_value('state', GLib.Variant('b', enabled))

        settings.set_value('high-pass-filter', GLib.Variant('b', high_pass))
        settings.set_value('extended-filter',
                           GLib.Variant('b', extended_filter))
        settings.set_value('delay-agnostic',
                           GLib.Variant('b', delay_agnostic))

        settings.set_value('target-level-dbfs',
                           GLib.Variant('i', target_level_dbfs))
        settings.set_value('compression-gain-db',
                           GLib.Variant('i', compression_gain_db))
        settings.set_value('limiter', GLib.Variant('b', limiter))

        settings.set_value('gain-control', GLib.Variant('b', gain_control))
        settings.set_value('gain-control-mode-adaptive',
                           GLib.Variant('b', gain_control_mode_adaptive))
        settings.set_value('gain-control-mode-fixed',
                           GLib.Variant('b', gain_control_mode_fixed))

        settings.set_value('echo-cancel', GLib.Variant('b', echo_cancel))
        settings.set_value('echo-suppression-level-low',
                           GLib.Variant('b', echo_suppression_level_low))
        settings.set_value('echo-suppression-level-moderate',
                           GLib.Variant('b', echo_suppression_level_moderate))
        settings.set_value('echo-suppression-level-high',
                           GLib.Variant('b', echo_suppression_level_high))

        settings.set_value('noise-suppression',
                           GLib.Variant('b', noise_suppression))
        settings.set_value('noise-suppression-level-low',
                           GLib.Variant('b', noise_suppression_level_low))
        settings.set_value('noise-suppression-level-moderate',
                           GLib.Variant('b', noise_suppression_level_moderate))
        settings.set_value('noise-suppression-level-high',
                           GLib.Variant('b', noise_suppression_level_high))
        settings.set_value('noise-suppression-level-very-high',
                           GLib.Variant('b',
                                        noise_suppression_level_very_high))

        settings.set_value('voice-detection',
                           GLib.Variant('b', voice_detection))
        settings.set_value('voice-detection-frame-size-ms',
                           GLib.Variant('i', voice_detection_frame_size))
        settings.set_value('voice-detection-likelihood-very-low',
                           GLib.Variant('b',
                                        voice_detection_likelihood_very_low))
        settings.set_value('voice-detection-likelihood-low',
                           GLib.Variant('b', voice_detection_likelihood_low))
        settings.set_value('voice-detection-likelihood-moderate',
                           GLib.Variant('b',
                                        voice_detection_likelihood_moderate))
        settings.set_value('voice-detection-likelihood-high',
                           GLib.Variant('b', voice_detection_likelihood_high))

    def save(self):
        self.add_section(self.settings_sourceoutputs, 'mic_webrtc')

    def load(self):
        self.load_section(self.settings_sourceoutputs, 'mic_webrtc')
