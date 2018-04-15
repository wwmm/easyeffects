# -*- coding: utf-8 -*-

from ctypes import (CFUNCTYPE, POINTER, Structure, byref, c_char_p, c_double,
                    c_int, c_uint8, c_uint32, c_uint64, c_size_t, c_void_p,
                    c_float, cdll, sizeof, cast)

lib = cdll.LoadLibrary("libpulse.so.0")


# enumerators constants

PA_CONTEXT_NOFLAGS = 0x0000
PA_CONTEXT_NOAUTOSPAWN = 0x0001
PA_CONTEXT_NOFAIL = 0x0002
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
PA_STREAM_UNCONNECTED = 0
PA_STREAM_CREATING = 1
PA_STREAM_READY = 2
PA_STREAM_FAILED = 3
PA_STREAM_TERMINATED = 4
PA_STREAM_NOFLAGS = 0x0000
PA_STREAM_START_CORKED = 0x0001
PA_STREAM_INTERPOLATE_TIMING = 0x0002
PA_STREAM_NOT_MONOTONIC = 0x0004
PA_STREAM_AUTO_TIMING_UPDATE = 0x0008
PA_STREAM_NO_REMAP_CHANNELS = 0x0010
PA_STREAM_NO_REMIX_CHANNELS = 0x0020
PA_STREAM_FIX_FORMAT = 0x0040
PA_STREAM_FIX_RATE = 0x0080
PA_STREAM_FIX_CHANNELS = 0x0100
PA_STREAM_DONT_MOVE = 0x0200
PA_STREAM_VARIABLE_RATE = 0x0400
PA_STREAM_PEAK_DETECT = 0x0800
PA_STREAM_START_MUTED = 0x1000
PA_STREAM_ADJUST_LATENCY = 0x2000
PA_STREAM_EARLY_REQUESTS = 0x4000
PA_STREAM_DONT_INHIBIT_AUTO_SUSPEND = 0x8000
PA_STREAM_START_UNMUTED = 0x10000
PA_STREAM_FAIL_ON_SUSPEND = 0x20000
PA_STREAM_RELATIVE_VOLUME = 0x40000
PA_STREAM_PASSTHROUGH = 0x80000

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
pa_stream_state_t = c_int
pa_stream_flags_t = c_int

uint32_t = c_uint32

# functions


def get_ref(obj):
    return byref(obj)


def get_c_void_p_ref():
    return byref(c_void_p())


def int_to_c_int(v):
    return c_int(v)


def int_to_c_size_t_ref(v):
    return byref(c_size_t(v))


def get_sizeof_float():
    return sizeof(c_float)


def cast_to_float(v):
    return cast(v, POINTER(POINTER(c_float)))


def cast_to_int(v):
    return cast(v, POINTER(c_int))


def cast_to_struct_ptr(void_ptr, s):
    return cast(void_ptr, POINTER(s))


def sample_spec_format_name(code):
    if code == PA_SAMPLE_U8:
        return 'u8'
    elif code == PA_SAMPLE_ALAW:
        return 'alaw'
    elif code == PA_SAMPLE_ULAW:
        return 'ulaw'
    elif code == PA_SAMPLE_S16LE:
        return 's16le'
    elif code == PA_SAMPLE_S16BE:
        return 's16be'
    elif code == PA_SAMPLE_FLOAT32LE:
        return 'float32le'
    elif code == PA_SAMPLE_FLOAT32BE:
        return 'float32be'
    elif code == PA_SAMPLE_S32LE:
        return 's32le'
    elif code == PA_SAMPLE_S32BE:
        return 's32Be'
    elif code == PA_SAMPLE_S24LE:
        return 's24le'
    elif code == PA_SAMPLE_S24BE:
        return 's24be'
    elif code == PA_SAMPLE_S24_32LE:
        return 's24_32le'
    elif code == PA_SAMPLE_S24_32BE:
        return 's24_32be'
    elif code == PA_SAMPLE_MAX:
        return 'pa_max'
    elif code == PA_SAMPLE_INVALID:
        return 'invalid'


# structures


class pa_mainloop_api(Structure):
    _fields_ = []


class pa_threaded_mainloop(Structure):
    _fields_ = []


class pa_context(Structure):
    _fields_ = []


class ctx_success_cb_data(Structure):
    _fields_ = [('operation', c_char_p)]


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


class pa_stream(Structure):
    _fields_ = []


class pa_buffer_attr(Structure):
    _fields_ = [('maxlength', c_uint32), ('tlength', c_uint32),
                ('prebuf', c_uint32), ('minreq', c_uint32),
                ('fragsize', c_uint32)]


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


class pa_source_output_info(Structure):
    _fields_ = [('index', c_uint32), ('name', c_char_p),
                ('owner_module', c_uint32), ('client', c_uint32),
                ('source', c_uint32), ('sample_spec', pa_sample_spec),
                ('channel_map', pa_channel_map), ('buffer_usec', pa_usec_t),
                ('source_usec', pa_usec_t), ('resample_method', c_char_p),
                ('driver', c_char_p), ('proplist', POINTER(pa_proplist)),
                ('corked', c_int), ('volume', pa_cvolume), ('mute', c_int),
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

pa_source_output_info_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                       POINTER(pa_source_output_info), c_int,
                                       c_void_p)

pa_context_subscribe_cb_t = CFUNCTYPE(None, POINTER(pa_context),
                                      pa_subscription_event_type_t, c_uint32,
                                      c_void_p)

pa_stream_notify_cb_t = CFUNCTYPE(None, POINTER(pa_stream), c_void_p)

pa_stream_request_cb_t = CFUNCTYPE(None, POINTER(pa_stream), c_size_t,
                                   c_void_p)

pa_stream_success_cb_t = CFUNCTYPE(None, POINTER(pa_stream), c_int, c_void_p)

# functions

pa_threaded_mainloop_new = lib.pa_threaded_mainloop_new
pa_threaded_mainloop_new.restype = POINTER(pa_threaded_mainloop)
pa_threaded_mainloop_new.argtypes = []

pa_threaded_mainloop_free = lib.pa_threaded_mainloop_free
pa_threaded_mainloop_free.restype = None
pa_threaded_mainloop_free.argtypes = [POINTER(pa_threaded_mainloop)]

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

pa_context_disconnect = lib.pa_context_disconnect
pa_context_disconnect.restype = None
pa_context_disconnect.argtypes = [POINTER(pa_context)]

pa_context_drain = lib.pa_context_drain
pa_context_drain.restype = POINTER(pa_operation)
pa_context_drain.argtypes = [POINTER(pa_context), pa_context_notify_cb_t,
                             c_void_p]

pa_context_unref = lib.pa_context_unref
pa_context_unref.restype = None
pa_context_unref.argtypes = [POINTER(pa_context)]

pa_context_get_module_info = lib.pa_context_get_module_info
pa_context_get_module_info.restype = POINTER(pa_operation)
pa_context_get_module_info.argtypes = [POINTER(pa_context), c_uint32,
                                       pa_module_info_cb_t, c_void_p]

pa_context_get_server = lib.pa_context_get_server
pa_context_get_server.restype = c_char_p
pa_context_get_server.argtypes = [POINTER(pa_context)]

pa_context_get_server_info = lib.pa_context_get_server_info
pa_context_get_server_info.restype = POINTER(pa_operation)
pa_context_get_server_info.argtypes = [POINTER(pa_context),
                                       pa_server_info_cb_t, c_void_p]

pa_context_get_server_protocol_version = \
    lib.pa_context_get_server_protocol_version

pa_context_get_server_protocol_version.restype = c_uint32
pa_context_get_server_protocol_version.argtypes = [POINTER(pa_context)]

pa_context_get_sink_info_by_index = lib.pa_context_get_sink_info_by_index
pa_context_get_sink_info_by_index.restype = POINTER(pa_operation)
pa_context_get_sink_info_by_index.argtypes = [POINTER(pa_context), c_uint32,
                                              pa_sink_info_cb_t, c_void_p]

pa_context_get_sink_info_by_name = lib.pa_context_get_sink_info_by_name
pa_context_get_sink_info_by_name.restype = POINTER(pa_operation)
pa_context_get_sink_info_by_name.argtypes = [POINTER(pa_context), c_char_p,
                                             pa_sink_info_cb_t, c_void_p]

pa_context_get_sink_info_list = lib.pa_context_get_sink_info_list
pa_context_get_sink_info_list.restype = POINTER(pa_operation)
pa_context_get_sink_info_list.argtypes = [POINTER(pa_context),
                                          pa_sink_info_cb_t, c_void_p]

pa_context_get_source_info_by_index = lib.pa_context_get_source_info_by_index
pa_context_get_source_info_by_index.restype = POINTER(pa_operation)
pa_context_get_source_info_by_index.argtypes = [POINTER(pa_context), c_uint32,
                                                pa_source_info_cb_t, c_void_p]

pa_context_get_source_info_by_name = lib.pa_context_get_source_info_by_name
pa_context_get_source_info_by_name.restype = POINTER(pa_operation)
pa_context_get_source_info_by_name.argtypes = [POINTER(pa_context), c_char_p,
                                               pa_source_info_cb_t, c_void_p]

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

pa_context_get_source_output_info = lib.pa_context_get_source_output_info
pa_context_get_source_output_info.restype = POINTER(pa_operation)
pa_context_get_source_output_info.argtypes = [POINTER(pa_context), c_uint32,
                                              pa_source_output_info_cb_t,
                                              c_void_p]

pa_context_get_source_output_info_list = \
    lib.pa_context_get_source_output_info_list
pa_context_get_source_output_info_list.restype = POINTER(pa_operation)
pa_context_get_source_output_info_list.argtypes = [POINTER(pa_context),
                                                   pa_source_output_info_cb_t,
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

pa_context_set_sink_input_mute = lib.pa_context_set_sink_input_mute
pa_context_set_sink_input_mute.restype = POINTER(pa_operation)
pa_context_set_sink_input_mute.argtypes = [POINTER(pa_context), c_uint32,
                                           c_int, pa_context_success_cb_t,
                                           c_void_p]

pa_context_set_source_output_volume = lib.pa_context_set_source_output_volume
pa_context_set_source_output_volume.restype = POINTER(pa_operation)
pa_context_set_source_output_volume.argtypes = [POINTER(pa_context), c_uint32,
                                                POINTER(pa_cvolume),
                                                pa_context_success_cb_t,
                                                c_void_p]

pa_context_set_source_output_mute = lib.pa_context_set_source_output_mute
pa_context_set_source_output_mute.restype = POINTER(pa_operation)
pa_context_set_source_output_mute.argtypes = [POINTER(pa_context), c_uint32,
                                              c_int, pa_context_success_cb_t,
                                              c_void_p]

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

pa_context_move_source_output_by_index = \
    lib.pa_context_move_source_output_by_index
pa_context_move_source_output_by_index.restype = POINTER(pa_operation)
pa_context_move_source_output_by_index.argtypes = [POINTER(pa_context),
                                                   c_uint32, c_uint32,
                                                   pa_context_success_cb_t,
                                                   c_void_p]

pa_context_move_source_output_by_name = \
    lib.pa_context_move_source_output_by_name
pa_context_move_source_output_by_name.restype = POINTER(pa_operation)
pa_context_move_source_output_by_name.argtypes = [POINTER(pa_context),
                                                  c_uint32, c_char_p,
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

pa_stream_new = lib.pa_stream_new
pa_stream_new.restype = POINTER(pa_stream)
pa_stream_new.argtypes = [POINTER(pa_context), c_char_p,
                          POINTER(pa_sample_spec), POINTER(pa_channel_map)]

pa_stream_set_monitor_stream = lib.pa_stream_set_monitor_stream
pa_stream_set_monitor_stream.restype = c_int
pa_stream_set_monitor_stream.argtypes = [POINTER(pa_stream), c_uint32]

pa_stream_set_read_callback = lib.pa_stream_set_read_callback
pa_stream_set_read_callback.restype = None
pa_stream_set_read_callback.argtypes = [POINTER(pa_stream),
                                        pa_stream_request_cb_t, c_void_p]

pa_stream_connect_record = lib.pa_stream_connect_record
pa_stream_connect_record.restype = c_int
pa_stream_connect_record.argtypes = [POINTER(pa_stream), c_char_p,
                                     POINTER(pa_buffer_attr),
                                     pa_stream_flags_t]

pa_stream_cork = lib.pa_stream_cork
pa_stream_cork.restype = POINTER(pa_operation)
pa_stream_cork.argtypes = [POINTER(pa_stream), c_int, pa_stream_success_cb_t,
                           c_void_p]

pa_stream_set_state_callback = lib.pa_stream_set_state_callback
pa_stream_set_state_callback.restype = None
pa_stream_set_state_callback.argtypes = [POINTER(pa_stream),
                                         pa_stream_notify_cb_t, c_void_p]

pa_stream_set_suspended_callback = lib.pa_stream_set_suspended_callback
pa_stream_set_suspended_callback.res = c_int
pa_stream_set_suspended_callback.argtypes = [POINTER(pa_stream),
                                             pa_stream_notify_cb_t, c_void_p]

pa_stream_get_state = lib.pa_stream_get_state
pa_stream_get_state.restype = pa_stream_state_t
pa_stream_get_state.argtypes = [POINTER(pa_stream)]

pa_stream_get_device_name = lib.pa_stream_get_device_name
pa_stream_get_device_name.restype = c_char_p
pa_stream_get_device_name.argtypes = [POINTER(pa_stream)]

pa_stream_get_index = lib.pa_stream_get_index
pa_stream_get_index.restype = c_uint32
pa_stream_get_index.argtypes = [POINTER(pa_stream)]

pa_stream_get_device_index = lib.pa_stream_get_device_index
pa_stream_get_device_index.restype = c_uint32
pa_stream_get_device_index.argtypes = [POINTER(pa_stream)]

pa_stream_peek = lib.pa_stream_peek
pa_stream_peek.restype = c_int
pa_stream_peek.argtypes = [POINTER(pa_stream), POINTER(c_void_p),
                           POINTER(c_size_t)]

pa_stream_drop = lib.pa_stream_drop
pa_stream_drop.restype = c_int
pa_stream_drop.argtypes = [POINTER(pa_stream)]

pa_stream_is_corked = lib.pa_stream_is_corked
pa_stream_is_corked.restype = c_int
pa_stream_is_corked.argtypes = [POINTER(pa_stream)]

pa_stream_is_suspended = lib.pa_stream_is_suspended
pa_stream_is_suspended.restype = c_int
pa_stream_is_suspended.argtypes = [POINTER(pa_stream)]

pa_stream_disconnect = lib.pa_stream_disconnect
pa_stream_disconnect.restype = c_int
pa_stream_disconnect.argtypes = [POINTER(pa_stream)]

pa_stream_unref = lib.pa_stream_unref
pa_stream_unref.restype = None
pa_stream_unref.argtypes = [POINTER(pa_stream)]

pa_sw_volume_from_dB = lib.pa_sw_volume_from_dB
pa_sw_volume_from_dB.restype = pa_volume_t
pa_sw_volume_from_dB.argtypes = [c_double]

pa_sw_volume_from_linear = lib.pa_sw_volume_from_linear
pa_sw_volume_from_linear.restype = pa_volume_t
pa_sw_volume_from_linear.argtypes = [c_double]

pa_sw_volume_to_dB = lib.pa_sw_volume_to_dB
pa_sw_volume_to_dB.restype = c_double
pa_sw_volume_to_dB.argtypes = [pa_volume_t]

pa_sw_volume_to_linear = lib.pa_sw_volume_to_linear
pa_sw_volume_to_linear.restype = c_double
pa_sw_volume_to_linear.argtypes = [pa_volume_t]

pa_operation_unref = lib.pa_operation_unref
pa_operation_unref.restype = None
pa_operation_unref.argtypes = [POINTER(pa_operation)]

pa_proplist_gets = lib.pa_proplist_gets
pa_proplist_gets.restype = c_char_p
pa_proplist_gets.argtypes = [POINTER(pa_proplist), c_char_p]

pa_proplist_to_string = lib.pa_proplist_to_string
pa_proplist_to_string.restype = c_char_p
pa_proplist_to_string.argtypes = [POINTER(pa_proplist)]
