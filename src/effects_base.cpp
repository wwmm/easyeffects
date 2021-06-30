#include "effects_base.hpp"

EffectsBase::EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      pm(pipe_manager),
      settings(Gio::Settings::create(schema)),
      global_settings(Gio::Settings::create("com.github.wwmm.easyeffects")) {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  autogain = std::make_shared<AutoGain>(log_tag, "com.github.wwmm.easyeffects.autogain", path + "autogain/", pm);

  bass_enhancer =
      std::make_shared<BassEnhancer>(log_tag, "com.github.wwmm.easyeffects.bassenhancer", path + "bassenhancer/", pm);

  compressor =
      std::make_shared<Compressor>(log_tag, "com.github.wwmm.easyeffects.compressor", path + "compressor/", pm);

  convolver = std::make_shared<Convolver>(log_tag, "com.github.wwmm.easyeffects.convolver", path + "convolver/", pm);

  crossfeed = std::make_shared<Crossfeed>(log_tag, "com.github.wwmm.easyeffects.crossfeed", path + "crossfeed/", pm);

  crystalizer =
      std::make_shared<Crystalizer>(log_tag, "com.github.wwmm.easyeffects.crystalizer", path + "crystalizer/", pm);

  deesser = std::make_shared<Deesser>(log_tag, "com.github.wwmm.easyeffects.deesser", path + "deesser/", pm);

  delay = std::make_shared<Delay>(log_tag, "com.github.wwmm.easyeffects.delay", path + "delay/", pm);

  echo_canceller = std::make_shared<EchoCanceller>(log_tag, "com.github.wwmm.easyeffects.echocanceller",
                                                   path + "echocanceller/", pm);

  equalizer = std::make_shared<Equalizer>(log_tag, "com.github.wwmm.easyeffects.equalizer", path + "equalizer/",
                                          "com.github.wwmm.easyeffects.equalizer.channel",
                                          path + "equalizer/leftchannel/", path + "equalizer/rightchannel/", pm);

  exciter = std::make_shared<Exciter>(log_tag, "com.github.wwmm.easyeffects.exciter", path + "exciter/", pm);

  filter = std::make_shared<Filter>(log_tag, "com.github.wwmm.easyeffects.filter", path + "filter/", pm);

  gate = std::make_shared<Gate>(log_tag, "com.github.wwmm.easyeffects.gate", path + "gate/", pm);

  limiter = std::make_shared<Limiter>(log_tag, "com.github.wwmm.easyeffects.limiter", path + "limiter/", pm);

  loudness = std::make_shared<Loudness>(log_tag, "com.github.wwmm.easyeffects.loudness", path + "loudness/", pm);

  maximizer = std::make_shared<Maximizer>(log_tag, "com.github.wwmm.easyeffects.maximizer", path + "maximizer/", pm);

  multiband_compressor = std::make_shared<MultibandCompressor>(
      log_tag, "com.github.wwmm.easyeffects.multibandcompressor", path + "multibandcompressor/", pm);

  multiband_gate = std::make_shared<MultibandGate>(log_tag, "com.github.wwmm.easyeffects.multibandgate",
                                                   path + "multibandgate/", pm);

  output_level =
      std::make_unique<OutputLevel>(log_tag, "com.github.wwmm.easyeffects.outputlevel", path + "outputlevel/", pm);

  pitch = std::make_shared<Pitch>(log_tag, "com.github.wwmm.easyeffects.pitch", path + "pitch/", pm);

  reverb = std::make_shared<Reverb>(log_tag, "com.github.wwmm.easyeffects.reverb", path + "reverb/", pm);

  rnnoise = std::make_shared<RNNoise>(log_tag, "com.github.wwmm.easyeffects.rnnoise", path + "rnnoise/", pm);

  spectrum = std::make_unique<Spectrum>(log_tag, "com.github.wwmm.easyeffects.spectrum",
                                        "/com/github/wwmm/easyeffects/spectrum/", pm);

  stereo_tools =
      std::make_shared<StereoTools>(log_tag, "com.github.wwmm.easyeffects.stereotools", path + "stereotools/", pm);

  plugins.insert(std::make_pair(autogain->name, autogain));
  plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer));
  plugins.insert(std::make_pair(compressor->name, compressor));
  plugins.insert(std::make_pair(convolver->name, convolver));
  plugins.insert(std::make_pair(crossfeed->name, crossfeed));
  plugins.insert(std::make_pair(crystalizer->name, crystalizer));
  plugins.insert(std::make_pair(deesser->name, deesser));
  plugins.insert(std::make_pair(delay->name, delay));
  plugins.insert(std::make_pair(echo_canceller->name, echo_canceller));
  plugins.insert(std::make_pair(equalizer->name, equalizer));
  plugins.insert(std::make_pair(exciter->name, exciter));
  plugins.insert(std::make_pair(filter->name, filter));
  plugins.insert(std::make_pair(gate->name, gate));
  plugins.insert(std::make_pair(limiter->name, limiter));
  plugins.insert(std::make_pair(loudness->name, loudness));
  plugins.insert(std::make_pair(maximizer->name, maximizer));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate));
  plugins.insert(std::make_pair(pitch->name, pitch));
  plugins.insert(std::make_pair(reverb->name, reverb));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools));

  for (auto& key : plugins | std::views::keys) {
    plugins_latency[key] = 0.0F;
  }

  compressor->latency.connect([=, this](float v) {
    plugins_latency[compressor->name] = v;

    broadcast_pipeline_latency();
  });

  convolver->latency.connect([=, this](float v) {
    plugins_latency[convolver->name] = v;

    broadcast_pipeline_latency();
  });

  crystalizer->latency.connect([=, this](float v) {
    plugins_latency[crystalizer->name] = v;

    broadcast_pipeline_latency();
  });

  echo_canceller->latency.connect([=, this](float v) {
    plugins_latency[echo_canceller->name] = v;

    broadcast_pipeline_latency();
  });

  equalizer->latency.connect([=, this](float v) {
    plugins_latency[equalizer->name] = v;

    broadcast_pipeline_latency();
  });

  loudness->latency.connect([=, this](float v) {
    plugins_latency[loudness->name] = v;

    broadcast_pipeline_latency();
  });

  maximizer->latency.connect([=, this](float v) {
    plugins_latency[maximizer->name] = v;

    broadcast_pipeline_latency();
  });

  pitch->latency.connect([=, this](float v) {
    plugins_latency[pitch->name] = v;

    broadcast_pipeline_latency();
  });

  settings->signal_changed("plugins").connect([&, this](auto key) { broadcast_pipeline_latency(); });
}

EffectsBase::~EffectsBase() {
  util::debug("effects_base: destroyed");
}

void EffectsBase::activate_filters() {
  for (auto& plugin : plugins | std::views::values) {
    plugin->set_active(true);
  }
}

void EffectsBase::deactivate_filters() {
  for (auto& plugin : plugins | std::views::values) {
    plugin->set_active(false);
  }
}

auto EffectsBase::get_pipeline_latency() -> float {
  auto selected_plugins = settings->get_string_array("plugins");

  float total = 0.0F;

  for (auto& name : selected_plugins) {
    total += plugins_latency[name];
  }

  total *= 1000.0F;

  return total;
}

void EffectsBase::broadcast_pipeline_latency() {
  float latency_value = get_pipeline_latency();

  util::debug(log_tag + "pipeline latency: " + std::to_string(latency_value) + " ms");

  Glib::signal_idle().connect_once([=, this] { pipeline_latency.emit(latency_value); });
}