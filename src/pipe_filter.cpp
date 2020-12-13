#include "pipe_filter.hpp"
#include "pipewire/filter.h"
#include "pipewire/keys.h"
#include "util.hpp"

namespace {

struct data;

struct port {
  struct data* data;
};

struct data {
  struct port *in_left, *in_right;
  struct port *out_left, *out_right;
};

void on_process(void* userdata, struct spa_io_position* position) {
  auto* d = static_cast<data*>(userdata);

  uint32_t n_samples = position->clock.duration;

  // pw_log_trace("do process %d", n_samples);
  util::warning("processing");

  auto* in_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_left, n_samples));
  auto* in_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->in_right, n_samples));

  auto* out_left = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_left, n_samples));
  auto* out_right = static_cast<float*>(pw_filter_get_dsp_buffer(d->out_right, n_samples));

  memcpy(out_left, in_left, n_samples * sizeof(float));
  memcpy(out_right, in_right, n_samples * sizeof(float));
}

void destroy_filter(void* data) {
  auto* pf = static_cast<PipeFilter*>(data);

  util::debug(pf->log_tag + "Destroying Pipewire filter...");

  spa_hook_remove(&pf->listener);
}

static const struct pw_filter_events filter_events = {
    PW_VERSION_FILTER_EVENTS,
    .destroy = destroy_filter,
    .process = on_process,
};

}  // namespace

PipeFilter::PipeFilter(pw_core* core) {
  auto* props_filter = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_filter, PW_KEY_MEDIA_TYPE, "Audio");
  pw_properties_set(props_filter, PW_KEY_MEDIA_CATEGORY, "Filter");
  pw_properties_set(props_filter, PW_KEY_MEDIA_ROLE, "DSP");
  pw_properties_set(props_filter, PW_KEY_NODE_AUTOCONNECT, "true");

  filter = pw_filter_new(core, "pe_filter", props_filter);

  pw_filter_add_listener(filter, &listener, &filter_events, this);

  data d{};

  // left channel input

  auto* props_in_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_left, PW_KEY_PORT_NAME, "pe_input_fl");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  d.in_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                    sizeof(port), props_in_left, nullptr, 0));

  // left channel input

  auto* props_in_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_in_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_in_right, PW_KEY_PORT_NAME, "pe_input_fr");
  pw_properties_set(props_in_right, "audio.channel", "FR");

  d.in_right = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_INPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                     sizeof(port), props_in_right, nullptr, 0));

  // left channel output

  auto* props_out_left = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_left, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_left, PW_KEY_PORT_NAME, "pe_output_fl");
  pw_properties_set(props_in_left, "audio.channel", "FL");

  d.out_left = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                     sizeof(port), props_out_left, nullptr, 0));

  auto* props_out_right = pw_properties_new(nullptr, nullptr);

  pw_properties_set(props_out_right, PW_KEY_FORMAT_DSP, "32 bit float mono audio");
  pw_properties_set(props_out_right, PW_KEY_PORT_NAME, "pe_output_fr");
  pw_properties_set(props_in_left, "audio.channel", "FR");

  d.out_right = static_cast<port*>(pw_filter_add_port(filter, PW_DIRECTION_OUTPUT, PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                                                      sizeof(port), props_out_right, nullptr, 0));

  if (pw_filter_connect(filter, PW_FILTER_FLAG_RT_PROCESS, nullptr, 0) < 0) {
    util::error(log_tag + "can not connect the filter to pipewire!");
  } else {
    util::debug(log_tag + "filter connected to pipewire");
  }
}

PipeFilter::~PipeFilter() {
  pw_filter_destroy(filter);
}
