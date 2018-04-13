# -*- coding: utf-8 -*-

import logging
import threading

import PulseEffects.libpulse as p
from gi.repository import GLib, GObject


class PulseManager(GObject.GObject):

    __gsignals__ = {
        'sink_input_added': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'sink_input_changed': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'sink_input_removed': (GObject.SignalFlags.RUN_FIRST, None, (int,)),
        'source_output_added': (GObject.SignalFlags.RUN_FIRST, None,
                                (object,)),
        'source_output_changed': (GObject.SignalFlags.RUN_FIRST, None,
                                  (object,)),
        'source_output_removed': (GObject.SignalFlags.RUN_FIRST, None, (int,)),
        'sink_added': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'sink_changed': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'sink_removed': (GObject.SignalFlags.RUN_FIRST, None, (int,)),
        'source_added': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'source_changed': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'source_removed': (GObject.SignalFlags.RUN_FIRST, None, (int,)),
        'stream_level_changed': (GObject.SignalFlags.RUN_FIRST, None,
                                 (int, float)),
        'new_default_sink': (GObject.SignalFlags.RUN_FIRST, None, (object,)),
        'new_default_source': (GObject.SignalFlags.RUN_FIRST, None, (object,))
    }

    use_default_sink = GObject.Property(type=bool, default=True)
    use_default_source = GObject.Property(type=bool, default=True)

    def __init__(self):
        GObject.GObject.__init__(self)

        self.event_ctx_ready = threading.Event()
        self.event_ctx_terminated = threading.Event()

        self.default_sink_idx = -1
        self.default_sink_name = ''
        self.default_sink_rate = -1
        self.default_sink_format = ''

        self.default_source_idx = -1
        self.default_source_name = ''
        self.default_source_rate = -1
        self.default_source_format = ''

        # these variables are used to get values inside pulseaudio's callbacks
        self.sink_is_loaded = False
        self.sink_owner_module = -1
        self.sink_idx = -1
        self.sink_rate = -1
        self.sink_format = ''
        self.sink_monitor_name = ''
        self.sink_monitor_idx = -1

        # these variables are used to get values inside pulseaudio's callbacks
        self.source_idx = -1
        self.source_rate = -1
        self.source_format = ''

        # we redirect sink inputs to this sink
        self.apps_sink_idx = -1
        self.apps_sink_owner_module = -1
        self.apps_sink_rate = -1
        self.apps_sink_format = ''
        self.apps_sink_monitor_name = ''
        self.apps_sink_monitor_idx = -1
        self.apps_sink_description = '\'PulseEffects(apps)\''

        # microphone processed output will be sent to this sink
        # we redirect source outputs to this sink monitor
        self.mic_sink_idx = -1
        self.mic_sink_owner_module = -1
        self.mic_sink_rate = -1
        self.mic_sink_format = ''
        self.mic_sink_monitor_name = ''
        self.mic_sink_monitor_idx = -1
        self.mic_sink_description = '\'PulseEffects(mic)\''

        self.max_volume = p.PA_VOLUME_NORM

        self.log = logging.getLogger('PulseEffects')

        self.log_tag = 'PM - '

        # it makes no sense to show some kind of apps. So we blacklist them
        self.app_blacklist = ['PulseEffects', 'pulseeffects',
                              'PulseEffectsWebrtcProbe', 'gsd-media-keys',
                              'GNOME Shell', 'libcanberra', 'gnome-pomodoro',
                              'PulseAudio Volume Control', 'Screenshot',
                              'speech-dispatcher']

        self.media_blacklist = ['pulsesink probe', 'bell-window-system',
                                'audio-volume-change', 'Peak detect',
                                'screen-capture']

        self.wrap_callbacks()

        # creating main loop and context
        self.main_loop = p.pa_threaded_mainloop_new()
        self.main_loop_api = p.pa_threaded_mainloop_get_api(self.main_loop)

        self.ctx = p.pa_context_new(self.main_loop_api, b'PulseEffects')

        p.pa_context_set_state_callback(self.ctx, self.ctx_notify_cb, None)

        p.pa_context_connect(self.ctx, None, p.PA_CONTEXT_NOFAIL, None)

        p.pa_threaded_mainloop_start(self.main_loop)

        self.event_ctx_ready.wait()

        self.get_server_info()
        self.get_default_sink_info()
        self.get_default_source_info()

    def wrap_callbacks(self):
        self.ctx_notify_cb = p.pa_context_notify_cb_t(self.ctx_notify_cb)
        self.server_info_cb = p.pa_server_info_cb_t(self.server_info_cb)
        self.sink_info_cb = p.pa_sink_info_cb_t(self.sink_info_cb)
        self.source_info_cb = p.pa_source_info_cb_t(self.source_info_cb)
        self.sink_input_info_cb = p.pa_sink_input_info_cb_t(
            self.sink_input_info_cb)
        self.source_output_info_cb = p.pa_source_output_info_cb_t(
            self.source_output_info_cb)
        self.ctx_success_cb = p.pa_context_success_cb_t(self.ctx_success_cb)
        self.subscribe_cb = p.pa_context_subscribe_cb_t(self.subscribe_cb)
        self.stream_state_cb = p.pa_stream_notify_cb_t(self.stream_state_cb)
        self.stream_read_cb = p.pa_stream_request_cb_t(self.stream_read_cb)

    def ctx_notify_cb(self, ctx, user_data):
        state = p.pa_context_get_state(ctx)

        if state == p.PA_CONTEXT_READY:
            self.log.debug(self.log_tag + 'pulseaudio context started')
            self.log.debug(self.log_tag + 'connected to server: ' +
                           p.pa_context_get_server(ctx).decode())
            self.log.debug(self.log_tag + 'server protocol version: ' +
                           str(p.pa_context_get_server_protocol_version(ctx)))

            p.pa_context_set_subscribe_callback(ctx, self.subscribe_cb,
                                                None)

            # subscribing to pulseaudio events

            subscription_mask = p.PA_SUBSCRIPTION_MASK_SINK_INPUT + \
                p.PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT + \
                p.PA_SUBSCRIPTION_MASK_SOURCE + \
                p.PA_SUBSCRIPTION_MASK_SINK + \
                p.PA_SUBSCRIPTION_MASK_SERVER

            p.pa_context_subscribe(ctx, subscription_mask,
                                   self.ctx_success_cb, None)

            self.event_ctx_ready.set()

        elif state == p.PA_CONTEXT_FAILED:
            self.log.critical(self.log_tag +
                              'failed to start pulseaudio context')
            self.log.critical(self.log_tag +
                              'unferencing pulseaudio context object')

            p.pa_context_unref(ctx)

        elif state == p.PA_CONTEXT_TERMINATED:
            self.log.debug(self.log_tag + 'pulseaudio context terminated')

            self.log.debug(self.log_tag +
                           'unferencing pulseaudio context object')

            p.pa_context_unref(ctx)

            self.event_ctx_terminated.set()

    def exit(self):
        self.unload_sinks()
        self.log.debug(self.log_tag + 'sinks unloaded')

        self.log.debug(self.log_tag + 'disconnecting pulseaudio context')
        p.pa_context_disconnect(self.ctx)

        self.event_ctx_terminated.wait()

        self.log.debug(self.log_tag + 'stopping pulseaudio threaded main loop')
        p.pa_threaded_mainloop_stop(self.main_loop)

        self.log.debug(self.log_tag + 'freeing pulseaudio main loop object')
        p.pa_threaded_mainloop_free(self.main_loop)

    def load_sink_info(self, name):
        p.pa_threaded_mainloop_lock(self.main_loop)

        o = p.pa_context_get_sink_info_by_name(self.ctx, name.encode(),
                                               self.sink_info_cb, None)

        while p.pa_operation_get_state(o) == p.PA_OPERATION_RUNNING:
            p.pa_threaded_mainloop_wait(self.main_loop)

        p.pa_operation_unref(o)

        p.pa_threaded_mainloop_unlock(self.main_loop)

    def load_source_info(self, name):
        p.pa_threaded_mainloop_lock(self.main_loop)

        o = p.pa_context_get_source_info_by_name(self.ctx, name.encode(),
                                                 self.source_info_cb, None)

        while p.pa_operation_get_state(o) == p.PA_OPERATION_RUNNING:
            p.pa_threaded_mainloop_wait(self.main_loop)

        p.pa_operation_unref(o)

        p.pa_threaded_mainloop_unlock(self.main_loop)

    def get_server_info(self):
        p.pa_threaded_mainloop_lock(self.main_loop)

        o = p.pa_context_get_server_info(self.ctx, self.server_info_cb, None)

        while p.pa_operation_get_state(o) == p.PA_OPERATION_RUNNING:
            p.pa_threaded_mainloop_wait(self.main_loop)

        p.pa_operation_unref(o)

        p.pa_threaded_mainloop_unlock(self.main_loop)

    def get_default_sink_info(self):
        self.load_sink_info(self.default_sink_name)

        self.default_sink_rate = self.sink_rate
        self.default_sink_idx = self.sink_idx
        self.default_sink_format = self.sink_format

        self.log.debug(self.log_tag +
                       'default pulseaudio sink audio format: ' +
                       str(self.default_sink_format))
        self.log.debug(self.log_tag +
                       'default pulseaudio sink sampling rate: ' +
                       str(self.default_sink_rate) +
                       ' Hz. We will use the same rate.')

    def get_default_source_info(self):
        self.load_source_info(self.default_source_name)

        self.default_source_rate = self.source_rate
        self.default_source_idx = self.source_idx
        self.default_source_format = self.source_format

        self.log.debug(self.log_tag +
                       'default pulseaudio source audio format: ' +
                       str(self.default_source_format))
        self.log.debug(self.log_tag +
                       'default pulseaudio source sampling rate: ' +
                       str(self.default_source_rate) +
                       ' Hz. We will use the same rate.')

    def server_info_cb(self, context, info, emit_signal):
        self.default_sink_name = info.contents.default_sink_name.decode()
        self.default_source_name = info.contents.default_source_name.decode()

        server_version = info.contents.server_version.decode()

        self.log.debug(self.log_tag + 'pulseaudio version: ' + server_version)
        self.log.debug(self.log_tag + 'default pulseaudio source: ' +
                       self.default_source_name)
        self.log.debug(self.log_tag + 'default pulseaudio sink: ' +
                       self.default_sink_name)

        if emit_signal:
            if (self.default_sink_name != 'PulseEffects_apps' and
                    self.use_default_sink):
                GLib.idle_add(self.emit, 'new_default_sink',
                              self.default_sink_name)

            if (self.default_source_name != 'PulseEffects_mic.monitor' and
                    self.use_default_source):
                GLib.idle_add(self.emit, 'new_default_source',
                              self.default_source_name)

        p.pa_threaded_mainloop_signal(self.main_loop, 0)

    def sink_info_cb(self, context, info, eol, emit_signal):
        if eol == -1:
            self.sink_is_loaded = False
        elif eol == 0:
            if info:
                self.sink_owner_module = info.contents.owner_module
                self.sink_idx = info.contents.index
                self.sink_rate = info.contents.sample_spec.rate

                sample_format = info.contents.sample_spec.format
                self.sink_format = p.sample_spec_format_name(sample_format)

                self.sink_monitor_name = info.contents.monitor_source_name\
                    .decode()

                self.sink_monitor_idx = info.contents.monitor_source

                name = info.contents.name.decode()

                if (emit_signal and name != 'PulseEffects_apps' and
                        name != 'PulseEffects_mic'):
                    description = info.contents.description.decode()

                    new_sink = {'name': name, 'idx': self.sink_idx,
                                'description': description,
                                'monitor_source_name': self.sink_monitor_name}

                    GLib.idle_add(self.emit, 'sink_added', new_sink)
        elif eol == 1:
            self.sink_is_loaded = True

        p.pa_threaded_mainloop_signal(self.main_loop, 0)

    def source_info_cb(self, context, info, eol, emit_signal):
        if info:
            self.source_idx = info.contents.index
            self.source_rate = info.contents.sample_spec.rate

            sample_format = info.contents.sample_spec.format
            self.source_format = p.sample_spec_format_name(sample_format)

            name = info.contents.name.decode()

            if (emit_signal and name != 'PulseEffects_apps.monitor' and
                    name != 'PulseEffects_mic.monitor'):
                description = info.contents.description.decode()

                new_sink = {'name': name, 'idx': self.source_idx,
                            'description': description}

                GLib.idle_add(self.emit, 'source_added', new_sink)

        p.pa_threaded_mainloop_signal(self.main_loop, 0)

    def load_sink(self, name, description, rate):
        self.sink_is_loaded = False

        self.load_sink_info(name)

        if not self.sink_is_loaded:
            args = []

            sink_properties = description
            sink_properties += 'device.class=\'sound\''

            args.append('sink_name=' + name)
            args.append('sink_properties=' + sink_properties)
            args.append('channels=2')
            args.append('rate=' + str(rate))

            argument = ' '.join(map(str, args)).encode('ascii')

            module = b'module-null-sink'

            def module_idx(context, idx, user_data):
                self.log.debug(self.log_tag + 'sink owner module index: ' +
                               str(idx))

                p.pa_threaded_mainloop_signal(self.main_loop, 0)

            self.module_idx_cb = p.pa_context_index_cb_t(module_idx)

            p.pa_threaded_mainloop_lock(self.main_loop)

            o = p.pa_context_load_module(self.ctx, module, argument,
                                         self.module_idx_cb, None)

            while p.pa_operation_get_state(o) == p.PA_OPERATION_RUNNING:
                p.pa_threaded_mainloop_wait(self.main_loop)

            p.pa_operation_unref(o)

            p.pa_threaded_mainloop_unlock(self.main_loop)

            self.load_sink_info(name)  # checking if sink was loaded

            if self.sink_is_loaded:
                return True
            else:
                return False
        else:
            return True

    def load_apps_sink(self):
        self.log.debug(self.log_tag +
                       'loading Pulseeffects applications sink...')

        name = 'PulseEffects_apps'
        description = 'device.description=' + self.apps_sink_description
        rate = self.default_sink_rate

        status = self.load_sink(name, description, rate)

        if status:
            self.apps_sink_idx = self.sink_idx
            self.apps_sink_owner_module = self.sink_owner_module
            self.apps_sink_rate = self.sink_rate
            self.apps_sink_format = self.sink_format
            self.apps_sink_monitor_name = self.sink_monitor_name
            self.apps_sink_monitor_idx = self.sink_monitor_idx

            self.log.debug(self.log_tag +
                           'Pulseeffects apps sink was successfully loaded')
            self.log.debug(self.log_tag + 'Pulseeffects apps sink index:' +
                           str(self.apps_sink_idx))
            self.log.debug(self.log_tag +
                           'Pulseeffects apps sink monitor name: ' +
                           self.sink_monitor_name)
        else:
            self.log.critical(self.log_tag + 'Could not load apps sink')

    def load_mic_sink(self):
        self.log.debug(self.log_tag +
                       'loading Pulseeffects microphone output sink...')

        name = 'PulseEffects_mic'
        description = 'device.description=' + self.mic_sink_description
        rate = self.default_source_rate

        status = self.load_sink(name, description, rate)

        if status:
            self.mic_sink_idx = self.sink_idx
            self.mic_sink_owner_module = self.sink_owner_module
            self.mic_sink_rate = self.sink_rate
            self.mic_sink_format = self.sink_format
            self.mic_sink_monitor_name = self.sink_monitor_name
            self.mic_sink_monitor_idx = self.sink_monitor_idx

            self.log.debug(self.log_tag +
                           'Pulseeffects mic sink was successfully loaded')
            self.log.debug(self.log_tag + 'Pulseeffects mic sink index:' +
                           str(self.mic_sink_idx))
            self.log.debug(self.log_tag +
                           'Pulseeffects mic sink monitor name: ' +
                           self.mic_sink_monitor_name)
        else:
            self.log.critical(self.log_tag + 'Could not load mic sink')

    def sink_input_info_cb(self, context, info, eol, user_data):
        if info:
            idx = info.contents.index
            proplist = info.contents.proplist

            app_name = p.pa_proplist_gets(proplist, b'application.name')
            media_name = p.pa_proplist_gets(proplist, b'media.name')
            icon_name = p.pa_proplist_gets(proplist, b'application.icon_name')

            if not app_name:
                app_name = ''
            else:
                app_name = app_name.decode()

            if not media_name:
                media_name = ''
            else:
                media_name = media_name.decode()

            if (app_name not in self.app_blacklist and
                    media_name not in self.media_blacklist):
                if not icon_name:
                    icon_name = 'audio-x-generic-symbolic'
                else:
                    icon_name = icon_name.decode()

                connected = False

                if info.contents.sink == self.apps_sink_idx:
                    connected = True

                volume = info.contents.volume
                audio_channels = volume.channels
                mute = info.contents.mute

                max_volume_linear = 100 * \
                    p.pa_cvolume_max(volume) / p.PA_VOLUME_NORM

                resample_method = info.contents.resample_method

                if resample_method:
                    resample_method = resample_method.decode()
                else:
                    resample_method = 'null'

                sample_spec = info.contents.sample_spec
                rate = sample_spec.rate
                sample_format = p.sample_spec_format_name(sample_spec.format)
                buffer_latency = info.contents.buffer_usec
                latency = info.contents.sink_usec
                corked = info.contents.corked

                new_input = {'index': idx, 'name': app_name,
                             'icon': icon_name, 'channels': audio_channels,
                             'volume': max_volume_linear, 'rate': rate,
                             'resampler': resample_method,
                             'format': sample_format, 'mute': mute,
                             'connected': connected,
                             'buffer_latency': buffer_latency,
                             'latency': latency, 'corked': corked}

                if user_data == 1:
                    GLib.idle_add(self.emit, 'sink_input_added', new_input)
                elif user_data == 2:
                    GLib.idle_add(self.emit, 'sink_input_changed', new_input)

    def source_output_info_cb(self, context, info, eol, user_data):
        if info:
            idx = info.contents.index
            proplist = info.contents.proplist

            app_name = p.pa_proplist_gets(proplist, b'application.name')
            media_name = p.pa_proplist_gets(proplist, b'media.name')
            icon_name = p.pa_proplist_gets(proplist, b'application.icon_name')

            if not app_name:
                app_name = ''
            else:
                app_name = app_name.decode()

            if not media_name:
                media_name = ''
            else:
                media_name = media_name.decode()

            if (app_name not in self.app_blacklist and
                    media_name not in self.media_blacklist):
                if not icon_name:
                    icon_name = 'audio-x-generic-symbolic'
                else:
                    icon_name = icon_name.decode()

                connected = False

                if info.contents.source == self.mic_sink_monitor_idx:
                    connected = True

                volume = info.contents.volume
                audio_channels = volume.channels
                mute = info.contents.mute

                max_volume_linear = 100 * \
                    p.pa_cvolume_max(volume) / p.PA_VOLUME_NORM

                resample_method = info.contents.resample_method

                if resample_method:
                    resample_method = resample_method.decode()
                else:
                    resample_method = 'null'

                sample_spec = info.contents.sample_spec
                rate = sample_spec.rate
                sample_format = p.sample_spec_format_name(sample_spec.format)
                buffer_latency = info.contents.buffer_usec
                latency = info.contents.source_usec
                corked = info.contents.corked

                new_output = {'index': idx, 'name': app_name,
                              'icon': icon_name, 'channels': audio_channels,
                              'volume': max_volume_linear, 'rate': rate,
                              'resampler': resample_method,
                              'format': sample_format, 'mute': mute,
                              'connected': connected,
                              'buffer_latency': buffer_latency,
                              'latency': latency, 'corked': corked}

                if user_data == 1:
                    GLib.idle_add(self.emit, 'source_output_added', new_output)
                elif user_data == 2:
                    GLib.idle_add(self.emit, 'source_output_changed',
                                  new_output)

    def find_sink_inputs(self):
        p.pa_context_get_sink_input_info_list(self.ctx,
                                              self.sink_input_info_cb,
                                              1)  # 1 for new

    def find_source_outputs(self):
        p.pa_context_get_source_output_info_list(self.ctx,
                                                 self.source_output_info_cb,
                                                 1)  # 1 for new

    def find_sinks(self):
        p.pa_context_get_sink_info_list(self.ctx,
                                        self.sink_info_cb,
                                        1)  # 1 for new

    def find_sources(self):
        p.pa_context_get_source_info_list(self.ctx,
                                          self.source_info_cb,
                                          1)  # 1 for new

    def move_sink_input_to_pulseeffects_sink(self, idx):
        p.pa_context_move_sink_input_by_index(self.ctx, idx,
                                              self.apps_sink_idx,
                                              self.ctx_success_cb, None)

    def move_sink_input_to_default_sink(self, idx):
        p.pa_context_move_sink_input_by_name(self.ctx, idx,
                                             self.default_sink_name.encode(),
                                             self.ctx_success_cb, None)

    def move_source_output_to_pulseeffects_source(self, idx):
        p.pa_context_move_source_output_by_index(self.ctx, idx,
                                                 self.mic_sink_monitor_idx,
                                                 self.ctx_success_cb, None)

    def move_source_output_to_default_source(self, idx):
        p.pa_context_move_source_output_by_name(self.ctx, idx,
                                                self.default_source_name
                                                .encode(),
                                                self.ctx_success_cb, None)

    def set_sink_input_volume(self, idx, audio_channels, value):
        cvolume = p.pa_cvolume()
        cvolume.channels = audio_channels

        raw_value = int(p.PA_VOLUME_NORM * value / 100)

        cvolume_ptr = p.pa_cvolume_set(p.get_ref(cvolume), audio_channels,
                                       raw_value)

        p.pa_context_set_sink_input_volume(self.ctx, idx, cvolume_ptr,
                                           self.ctx_success_cb, None)

    def set_sink_input_mute(self, idx, mute_state):
        p.pa_context_set_sink_input_mute(self.ctx, idx, mute_state,
                                         self.ctx_success_cb, None)

    def set_source_output_volume(self, idx, audio_channels, value):
        cvolume = p.pa_cvolume()
        cvolume.channels = audio_channels

        raw_value = int(p.PA_VOLUME_NORM * value / 100)

        cvolume_ptr = p.pa_cvolume_set(p.get_ref(cvolume), audio_channels,
                                       raw_value)

        p.pa_context_set_source_output_volume(self.ctx, idx, cvolume_ptr,
                                              self.ctx_success_cb, None)

    def set_source_output_mute(self, idx, mute_state):
        p.pa_context_set_source_output_mute(self.ctx, idx, mute_state,
                                            self.ctx_success_cb, None)

    def stream_state_cb(self, stream, idx):
        state = p.pa_stream_get_state(stream)

        if not idx:  # zero is interpreted by python as None
            idx = 0

        if state == p.PA_STREAM_FAILED:
            self.log.debug(self.log_tag + 'volume meter stream for app ' +
                           str(idx) + ' has failed')

            p.pa_stream_disconnect(stream)
            p.pa_stream_unref(stream)
        elif state == p.PA_STREAM_READY:
            self.log.debug(self.log_tag + 'volume meter stream for app ' +
                           str(idx) + ' is ready')
        elif state == p.PA_STREAM_TERMINATED:
            self.log.debug(self.log_tag + 'volume meter stream for app ' +
                           str(idx) + ' was terminated')

            p.pa_stream_unref(stream)
        elif state == p.PA_STREAM_UNCONNECTED:
            self.log.debug(self.log_tag + 'volume meter stream for app ' +
                           str(idx) + ' is unconnected')

            p.pa_stream_unref(stream)
        elif state == p.PA_STREAM_CREATING:
            self.log.debug(self.log_tag + 'volume meter stream for app ' +
                           str(idx) + ' is being created')

    def stream_read_cb(self, stream, length, idx):
        data = p.get_c_void_p_ref()
        clength = p.int_to_c_size_t_ref(length)

        if p.pa_stream_peek(stream, data, clength) < 0:
            self.log.warn(self.log_tag + 'failed to read stream' + str(idx) +
                          'data')
            return

        d = p.cast_to_float(data)

        # according to pulseaudio docs:
        # NULL data means either a hole or empty buffer.
        # Only drop the stream when there is a hole (length > 0)
        if not d.contents:
            if p.cast_to_int(clength):
                p.pa_stream_drop(stream)
                return
            else:
                return

        v = d.contents.contents.value

        p.pa_stream_drop(stream)

        if v < 0:
            v = 0.0
        elif v > 1:
            v = 1.0

        if not idx:
            idx = 0

        GLib.idle_add(self.emit, 'stream_level_changed', idx, v)

    def create_stream(self, source_name, app_idx, app_name, monitor_idx):
        ss = p.pa_sample_spec()
        ss.channels = 1
        ss.format = p.PA_SAMPLE_FLOAT32LE
        ss.rate = 10

        stream_name = app_name + ' - Level Meter Stream'

        stream = p.pa_stream_new(self.ctx, stream_name.encode('utf-8'),
                                 p.get_ref(ss), None)

        p.pa_stream_set_state_callback(stream, self.stream_state_cb, app_idx)

        if monitor_idx != -1:
            p.pa_stream_set_monitor_stream(stream, monitor_idx)

        p.pa_stream_set_read_callback(stream, self.stream_read_cb, app_idx)

        flags = p.PA_STREAM_PEAK_DETECT | p.PA_STREAM_DONT_MOVE

        p.pa_stream_connect_record(stream, source_name, None, flags)

        return stream

    def disconnect_stream(self, stream):
        p.pa_stream_disconnect(stream)

    def subscribe_cb(self, context, event_value, idx, user_data):
        if not idx:
            idx = 0

        event_facility = event_value & p.PA_SUBSCRIPTION_EVENT_FACILITY_MASK

        if event_facility == p.PA_SUBSCRIPTION_EVENT_SINK_INPUT:
            event_type = event_value & p.PA_SUBSCRIPTION_EVENT_TYPE_MASK

            if event_type == p.PA_SUBSCRIPTION_EVENT_NEW:
                p.pa_context_get_sink_input_info(context, idx,
                                                 self.sink_input_info_cb,
                                                 1)  # 1 for new
            elif event_type == p.PA_SUBSCRIPTION_EVENT_CHANGE:
                p.pa_context_get_sink_input_info(context, idx,
                                                 self.sink_input_info_cb,
                                                 2)  # 2 for changes
            elif event_type == p.PA_SUBSCRIPTION_EVENT_REMOVE:
                GLib.idle_add(self.emit, 'sink_input_removed', idx)
        elif event_facility == p.PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT:
            event_type = event_value & p.PA_SUBSCRIPTION_EVENT_TYPE_MASK

            if event_type == p.PA_SUBSCRIPTION_EVENT_NEW:
                p.pa_context_get_source_output_info(context, idx,
                                                    self.source_output_info_cb,
                                                    1)  # 1 for new
            elif event_type == p.PA_SUBSCRIPTION_EVENT_CHANGE:
                p.pa_context_get_source_output_info(context, idx,
                                                    self.source_output_info_cb,
                                                    2)  # 1 for new
            elif event_type == p.PA_SUBSCRIPTION_EVENT_REMOVE:
                GLib.idle_add(self.emit, 'source_output_removed', idx)
        elif event_facility == p.PA_SUBSCRIPTION_EVENT_SOURCE:
            event_type = event_value & p.PA_SUBSCRIPTION_EVENT_TYPE_MASK

            if event_type == p.PA_SUBSCRIPTION_EVENT_NEW:
                p.pa_context_get_source_info_by_index(self.ctx, idx,
                                                      self.source_info_cb,
                                                      1)  # 1 for new
            elif event_type == p.PA_SUBSCRIPTION_EVENT_CHANGE:
                pass
            elif event_type == p.PA_SUBSCRIPTION_EVENT_REMOVE:
                GLib.idle_add(self.emit, 'source_removed', idx)
        elif event_facility == p.PA_SUBSCRIPTION_EVENT_SINK:
            event_type = event_value & p.PA_SUBSCRIPTION_EVENT_TYPE_MASK

            if event_type == p.PA_SUBSCRIPTION_EVENT_NEW:
                p.pa_context_get_sink_info_by_index(self.ctx, idx,
                                                    self.sink_info_cb,
                                                    1)  # 1 for new
            elif event_type == p.PA_SUBSCRIPTION_EVENT_CHANGE:
                pass
            elif event_type == p.PA_SUBSCRIPTION_EVENT_REMOVE:
                GLib.idle_add(self.emit, 'sink_removed', idx)
        elif event_facility == p.PA_SUBSCRIPTION_EVENT_SERVER:
            event_type = event_value & p.PA_SUBSCRIPTION_EVENT_TYPE_MASK

            if event_type == p.PA_SUBSCRIPTION_EVENT_CHANGE:
                p.pa_context_get_server_info(context, self.server_info_cb, 1)

    def ctx_success_cb(self, context, success, user_data):
        if not success:
            self.log.critical(self.log_tag + 'context operation failed!!')

    def unload_sinks(self):
        p.pa_context_unload_module(self.ctx, self.apps_sink_owner_module,
                                   self.ctx_success_cb, None)

        p.pa_context_unload_module(self.ctx, self.mic_sink_owner_module,
                                   self.ctx_success_cb, None)
