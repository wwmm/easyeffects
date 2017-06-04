# -*- coding: utf-8 -*-

from ctypes import (CFUNCTYPE, POINTER, Structure, byref, c_char_p, c_double,
                    c_int, c_uint8, c_uint32, c_uint64, c_void_p, cdll)

lib = cdll.LoadLibrary("libpulse.so.0")


def get_pointer(obj):
    return byref(obj)


# enumerators

PA_CONTEXT_UNCONNECTED = 0
PA_CONTEXT_CONNECTING = 1
PA_CONTEXT_AUTHORIZING = 2
PA_CONTEXT_SETTING_NAME = 3
PA_CONTEXT_READY = 4
PA_CONTEXT_FAILED = 5
PA_CONTEXT_TERMINATED = 6
PA_OPERATION_RUNNING = 0
PA_OPERATION_DONE = 1
PA_OPERATION_CANCELLED = 2
PA_CHANNELS_MAX = 32
PA_SINK_NOFLAGS = 0
PA_SINK_HW_VOLUME_CTRL = 1
PA_SINK_LATENCY = 2
PA_SINK_HARDWARE = 3
PA_SINK_NETWORK = 4
PA_SINK_HW_MUTE_CTRL = 5
PA_SINK_DECIBEL_VOLUME = 6
PA_SINK_FLAT_VOLUME = 7
PA_SINK_DYNAMIC_LATENCY = 8
PA_SINK_SET_FORMATS = 9
PA_SINK_INVALID_STATE = 0
PA_SINK_RUNNING = 1
PA_SINK_IDLE = 2
PA_SINK_SUSPENDED = 3
PA_ENCODING_ANY = 0
PA_ENCODING_PCM = 1
PA_ENCODING_AC3_IEC61937 = 2
PA_ENCODING_EAC3_IEC61937 = 3
PA_ENCODING_MPEG_IEC61937 = 4
PA_ENCODING_DTS_IEC61937 = 5
PA_ENCODING_MPEG2_AAC_IEC61937 = 6
PA_ENCODING_MAX = 7
PA_ENCODING_INVALID = 8
PA_SAMPLE_U8 = 0
PA_SAMPLE_ALAW = 1
PA_SAMPLE_ULAW = 2
PA_SAMPLE_S16LE = 3
PA_SAMPLE_S16BE = 4
PA_SAMPLE_FLOAT32LE = 5
PA_SAMPLE_FLOAT32BE = 6
PA_SAMPLE_S32LE = 7
PA_SAMPLE_S32BE = 8
PA_SAMPLE_S24LE = 9
PA_SAMPLE_S24BE = 10
PA_SAMPLE_S24_32LE = 11
PA_SAMPLE_S24_32BE = 12
PA_SAMPLE_MAX = 13
PA_SAMPLE_INVALID = -1
PA_SUBSCRIPTION_EVENT_SINK = 0x0000
PA_SUBSCRIPTION_EVENT_SOURCE = 0x0001
PA_SUBSCRIPTION_EVENT_SINK_INPUT = 0x0002
PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT = 0x0003
PA_SUBSCRIPTION_EVENT_MODULE = 0x0004
PA_SUBSCRIPTION_EVENT_CLIENT = 0x0005
PA_SUBSCRIPTION_EVENT_SAMPLE_CACHE = 0x0006
PA_SUBSCRIPTION_EVENT_SERVER = 0x0007
PA_SUBSCRIPTION_EVENT_CARD = 0x0009
PA_SUBSCRIPTION_EVENT_FACILITY_MASK = 0x000F
PA_SUBSCRIPTION_EVENT_NEW = 0x0000
PA_SUBSCRIPTION_EVENT_CHANGE = 0x0010
PA_SUBSCRIPTION_EVENT_REMOVE = 0x0020
PA_SUBSCRIPTION_EVENT_TYPE_MASK = 0x0030
PA_SUBSCRIPTION_MASK_NULL = 0x0000
PA_SUBSCRIPTION_MASK_SINK = 0x0001
PA_SUBSCRIPTION_MASK_SOURCE = 0x0002
PA_SUBSCRIPTION_MASK_SINK_INPUT = 0x0004
PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT = 0x0008
PA_SUBSCRIPTION_MASK_MODULE = 0x0010
PA_SUBSCRIPTION_MASK_CLIENT = 0x0020
PA_SUBSCRIPTION_MASK_SAMPLE_CACHE = 0x0040
PA_SUBSCRIPTION_MASK_SERVER = 0x0080
PA_SUBSCRIPTION_MASK_CARD = 0x0200
PA_SUBSCRIPTION_MASK_ALL = 0x02ff
PA_VOLUME_MUTED = 0
PA_VOLUME_NORM = 0x10000

pa_io_event_flags_t = c_int
pa_context_flags_t = c_int
pa_subscription_mask_t = c_int
pa_subscription_event_type_t = c_int
pa_sample_format_t = c_int
pa_operation_state_t = c_int
pa_channel_position_t = c_int
pa_volume_t = c_uint32
pa_usec_t = c_uint64
pa_sink_flags_t = c_int
pa_sink_state_t = c_int
pa_encoding_t = c_int

# structures


class pa_mainloop_api(Structure):
    _fields_ = []


class pa_threaded_mainloop(Structure):
    _fields_ = []


class pa_context(Structure):
    _fields_ = []


class pa_spawn_api(Structure):
    _fields_ = []


class pa_proplist(Structure):
    _fields_ = []


class pa_operation(Structure):
    _fields_ = []


class pa_sample_spec(Structure):
    _fields_ = [('format', pa_sample_format_t),
                ('rate', c_uint32), ('channels', c_uint8)]


class pa_channel_map(Structure):
    _fields_ = [('channels', c_uint8),
                ('map', pa_channel_position_t * PA_CHANNELS_MAX)]


class pa_cvolume(Structure):
    _fields_ = [('channels', c_uint8),
                ('values', pa_volume_t * PA_CHANNELS_MAX)]


class pa_sink_port_info(Structure):
    _fields_ = [('name', c_char_p), ('description', c_char_p),
                ('priority', c_uint32), ('available', c_int)]


class pa_format_info(Structure):
    _fields_ = [('encoding', pa_encoding_t), ('plist', POINTER(pa_proplist))]


class pa_server_info(Structure):
    _fields_ = [('user_name', c_char_p), ('host_name', c_char_p),
                ('server_version', c_char_p), ('server_name', c_char_p),
                ('sample_spec', pa_sample_spec),
                ('default_sink_name', c_char_p),
                ('default_source_name', c_char_p)]


class pa_client_info(Structure):
    _fields_ = [('index', c_uint32), ('name', c_char_p),
                ('owner_module', c_uint32), ('driver', c_char_p),
                ('proplist', POINTER(pa_proplist))]


class pa_module_info(Structure):
    _fields_ = [('index', c_uint32), ('name', c_char_p),
                ('argument', c_char_p), ('n_used', c_uint32),
                ('proplist', POINTER(pa_proplist))]


class pa_sink_info(Structure):
    _fields_ = [('name', c_char_p), ('index', c_uint32),
                ('description', c_char_p), ('sample_spec', pa_sample_spec),
                ('channel_map', pa_channel_map), ('owner_module', c_uint32),
                ('volume', pa_cvolume), ('mute', c_int),
                ('monitor_source', c_uint32),
                ('monitor_source_name', c_char_p), ('latency', pa_usec_t),
                ('driver', c_char_p), ('flags', pa_sink_flags_t),
                ('proplist', POINTER(pa_proplist)),
                ('configured_latency', pa_usec_t),
                ('base_volume', pa_volume_t), ('state', pa_sink_state_t),
                ('n_volume_steps', c_uint32), ('card', c_uint32),
                ('n_ports', c_uint32),
                ('ports', POINTER(POINTER(pa_sink_port_info))),
                ('active_port', POINTER(pa_sink_port_info)),
                ('n_formats', c_uint8),
                ('formats', POINTER(POINTER(pa_format_info)))]


class pa_source_info(Structure):
    _fields_ = [('name', c_char_p), ('index', c_uint32),
                ('description', c_char_p), ('sample_spec', pa_sample_spec),
                ('channel_map', pa_channel_map), ('owner_module', c_uint32),
                ('volume', pa_cvolume), ('mute', c_int),
                ('monitor_of_sink', c_uint32),
                ('monitor_of_sink_name', c_char_p), ('latency', pa_usec_t),
                ('driver', c_char_p), ('flags', pa_sink_flags_t),
                ('proplist', POINTER(pa_proplist)),
                ('configured_latency', pa_usec_t),
                ('base_volume', pa_volume_t), ('state', pa_sink_state_t),
                ('n_volume_steps', c_uint32), ('card', c_uint32),
                ('n_ports', c_uint32),
                ('ports', POINTER(POINTER(pa_sink_port_info))),
                ('active_port', POINTER(pa_sink_port_info)),
                ('n_formats', c_uint8),
                ('formats', POINTER(POINTER(pa_format_info)))]


class pa_sink_input_info(Structure):
    _fields_ = [('index', c_uint32), ('name', c_char_p),
                ('owner_module', c_uint32), ('client', c_uint32),
                ('sink', c_uint32), ('sample_spec', pa_sample_spec),
                ('channel_map', pa_channel_map), ('volume', pa_cvolume),
                ('buffer_usec', pa_usec_t), ('sink_usec', pa_usec_t),
                ('resample_method', c_char_p),
                ('driver', c_char_p), ('mute', c_int),
                ('proplist', POINTER(pa_proplist)), ('corked', c_int),
                ('has_volume', c_int), ('volume_writable', c_int),
                ('format', POINTER(pa_format_info))]


# callback types
pa_context_notify_cb_t = CFUNCTYPE(None, POINTER(pa_context), c_void_p)
pa_server_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                POINTER(pa_server_info), c_void_p)
pa_context_index_cb_t = CFUNCTYPE(None, POINTER(pa_context), c_uint32,
                                  c_void_p)

pa_module_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                POINTER(pa_module_info), c_int, c_void_p)

pa_sink_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                              POINTER(pa_sink_info), c_int, c_void_p)

pa_context_success_cb_t = CFUNCTYPE(None, POINTER(pa_context), c_int, c_void_p)

pa_client_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                POINTER(pa_client_info), c_int, c_void_p)

pa_source_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                POINTER(pa_source_info), c_int, c_void_p)

pa_sink_input_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                    POINTER(pa_sink_input_info), c_int,
                                    c_void_p)

pa_context_subscribe_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                      pa_subscription_event_type_t, c_uint32,
                                      c_void_p)

# functions

pa_threaded_mainloop_new = lib.pa_threaded_mainloop_new
pa_threaded_mainloop_new.restype = POINTER(pa_threaded_mainloop)
pa_threaded_mainloop_new.argtypes = []

pa_threaded_mainloop_get_api = lib.pa_threaded_mainloop_get_api
pa_threaded_mainloop_get_api.restype = POINTER(pa_mainloop_api)
pa_threaded_mainloop_get_api.argtypes = [POINTER(pa_threaded_mainloop)]

pa_threaded_mainloop_start = lib.pa_threaded_mainloop_start
pa_threaded_mainloop_start.restype = c_int
pa_threaded_mainloop_start.argtypes = [POINTER(pa_threaded_mainloop)]

pa_threaded_mainloop_stop = lib.pa_threaded_mainloop_stop
pa_threaded_mainloop_stop.restype = c_int
pa_threaded_mainloop_stop.argtypes = [POINTER(pa_threaded_mainloop)]

pa_threaded_mainloop_signal = lib.pa_threaded_mainloop_signal
pa_threaded_mainloop_signal.restype = None
pa_threaded_mainloop_signal.argtypes = [POINTER(pa_threaded_mainloop), c_int]

pa_threaded_mainloop_wait = lib.pa_threaded_mainloop_wait
pa_threaded_mainloop_wait.restype = None
pa_threaded_mainloop_wait.argtypes = [POINTER(pa_threaded_mainloop)]

pa_threaded_mainloop_lock = lib.pa_threaded_mainloop_lock
pa_threaded_mainloop_lock.restype = None
pa_threaded_mainloop_lock.argtypes = [POINTER(pa_threaded_mainloop)]

pa_threaded_mainloop_unlock = lib.pa_threaded_mainloop_unlock
pa_threaded_mainloop_unlock.restype = None
pa_threaded_mainloop_unlock.argtypes = [POINTER(pa_threaded_mainloop)]

pa_context_new = lib.pa_context_new
pa_context_new.restype = POINTER(pa_context)
pa_context_new.argtypes = [POINTER(pa_mainloop_api), c_char_p]

pa_context_get_state = lib.pa_context_get_state
pa_context_get_state.restype = c_int
pa_context_get_state.argtypes = [POINTER(pa_context)]

pa_context_set_state_callback = lib.pa_context_set_state_callback
pa_context_set_state_callback.restype = None
pa_context_set_state_callback.argtypes = [
    POINTER(pa_context), pa_context_notify_cb_t, c_void_p]

pa_context_connect = lib.pa_context_connect
pa_context_connect.restype = c_int
pa_context_connect.argtypes = [
    POINTER(pa_context), c_char_p, pa_context_flags_t, POINTER(pa_spawn_api)]

pa_context_get_module_info = lib.pa_context_get_module_info
pa_context_get_module_info.restype = POINTER(pa_operation)
pa_context_get_module_info.argtypes = [POINTER(pa_context), c_uint32,
                                       pa_module_info_cb_t, c_void_p]

pa_context_get_server_info = lib.pa_context_get_server_info
pa_context_get_server_info.restype = POINTER(pa_operation)
pa_context_get_server_info.argtypes = [POINTER(pa_context),
                                       pa_server_info_cb_t, c_void_p]

pa_context_get_sink_info_by_name = lib.pa_context_get_sink_info_by_name
pa_context_get_sink_info_by_name.restype = POINTER(pa_operation)
pa_context_get_sink_info_by_name.argtypes = [POINTER(pa_context), c_char_p,
                                             pa_sink_info_cb_t, c_void_p]

pa_context_get_source_info_list = lib.pa_context_get_source_info_list
pa_context_get_source_info_list.restype = POINTER(pa_operation)
pa_context_get_source_info_list.argtypes = [POINTER(pa_context),
                                            pa_source_info_cb_t, c_void_p]

pa_context_get_sink_input_info = lib.pa_context_get_sink_input_info
pa_context_get_sink_input_info.restype = POINTER(pa_operation)
pa_context_get_sink_input_info.argtypes = [POINTER(pa_context), c_uint32,
                                           pa_sink_input_info_cb_t, c_void_p]

pa_context_get_sink_input_info_list = lib.pa_context_get_sink_input_info_list
pa_context_get_sink_input_info_list.restype = POINTER(pa_operation)
pa_context_get_sink_input_info_list.argtypes = [POINTER(pa_context),
                                                pa_sink_input_info_cb_t,
                                                c_void_p]

pa_context_get_client_info_list = lib.pa_context_get_client_info_list
pa_context_get_client_info_list.restype = POINTER(pa_operation)
pa_context_get_client_info_list.argtypes = [POINTER(pa_context),
                                            pa_client_info_cb_t, c_void_p]

pa_operation_get_state = lib.pa_operation_get_state
pa_operation_get_state.restype = pa_operation_state_t
pa_operation_get_state.argtypes = [POINTER(pa_operation)]

pa_context_set_sink_input_volume = lib.pa_context_set_sink_input_volume
pa_context_set_sink_input_volume.restype = POINTER(pa_operation)
pa_context_set_sink_input_volume.argtypes = [POINTER(pa_context), c_uint32,
                                             POINTER(pa_cvolume),
                                             pa_context_success_cb_t, c_void_p]

pa_context_load_module = lib.pa_context_load_module
pa_context_load_module.restype = POINTER(pa_operation)
pa_context_load_module.argtypes = [POINTER(pa_context), c_char_p, c_char_p,
                                   pa_context_index_cb_t, c_void_p]

pa_context_move_sink_input_by_index = lib.pa_context_move_sink_input_by_index
pa_context_move_sink_input_by_index.restype = POINTER(pa_operation)
pa_context_move_sink_input_by_index.argtypes = [POINTER(pa_context), c_uint32,
                                                c_uint32,
                                                pa_context_success_cb_t,
                                                c_void_p]

pa_context_move_sink_input_by_name = lib.pa_context_move_sink_input_by_name
pa_context_move_sink_input_by_name.restype = POINTER(pa_operation)
pa_context_move_sink_input_by_name.argtypes = [POINTER(pa_context), c_uint32,
                                               c_char_p,
                                               pa_context_success_cb_t,
                                               c_void_p]

pa_context_subscribe = lib.pa_context_subscribe
pa_context_subscribe.restype = POINTER(pa_operation)
pa_context_subscribe.argtypes = [POINTER(pa_context), pa_subscription_mask_t,
                                 pa_context_success_cb_t, c_void_p]

pa_context_set_subscribe_callback = lib.pa_context_set_subscribe_callback
pa_context_set_subscribe_callback.restype = None
pa_context_set_subscribe_callback.argtypes = [POINTER(pa_context),
                                              pa_context_subscribe_cb_t,
                                              c_void_p]

pa_context_unload_module = lib.pa_context_unload_module
pa_context_unload_module.restype = POINTER(pa_operation)
pa_context_unload_module.argtypes = [POINTER(pa_context), c_int,
                                     pa_context_success_cb_t, c_void_p]

pa_cvolume_min = lib.pa_cvolume_min
pa_cvolume_min.restype = pa_volume_t
pa_cvolume_min.argtypes = [POINTER(pa_cvolume)]

pa_cvolume_max = lib.pa_cvolume_max
pa_cvolume_max.restype = pa_volume_t
pa_cvolume_max.argtypes = [POINTER(pa_cvolume)]

pa_cvolume_scale = lib.pa_cvolume_scale
pa_cvolume_scale.restype = POINTER(pa_cvolume)
pa_cvolume_scale.argtypes = [POINTER(pa_cvolume), pa_volume_t]

pa_cvolume_set = lib.pa_cvolume_set
pa_cvolume_set.restype = POINTER(pa_cvolume)
pa_cvolume_set.argtypes = [POINTER(pa_cvolume), c_uint8, pa_volume_t]

pa_sw_volume_from_dB = lib.pa_sw_volume_from_dB
pa_sw_volume_from_dB.restype = pa_volume_t
pa_sw_volume_from_dB.argtypes = [c_double]

pa_sw_volume_to_dB = lib.pa_sw_volume_to_dB
pa_sw_volume_to_dB.restype = c_double
pa_sw_volume_to_dB.argtypes = [pa_volume_t]

pa_operation_unref = lib.pa_operation_unref
pa_operation_unref.restype = None
pa_operation_unref.argtypes = [POINTER(pa_operation)]

pa_proplist_gets = lib.pa_proplist_gets
pa_proplist_gets.restype = c_char_p
pa_proplist_gets.argtypes = [POINTER(pa_proplist), c_char_p]

pa_proplist_to_string = lib.pa_proplist_to_string
pa_proplist_to_string.restype = c_char_p
pa_proplist_to_string.argtypes = [POINTER(pa_proplist)]
