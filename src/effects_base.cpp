#include "effects_base.hpp"

EffectsBase::EffectsBase(std::string tag, const std::string& schema, PipeManager* pipe_manager)
    : log_tag(std::move(tag)),
      pm(pipe_manager),
      settings(Gio::Settings::create(schema)),
      global_settings(Gio::Settings::create("com.github.wwmm.pulseeffects")) {
  std::string path = "/" + schema + "/";

  std::replace(path.begin(), path.end(), '.', '/');

  autogain = std::make_shared<AutoGain>(log_tag, "com.github.wwmm.pulseeffects.autogain", path + "autogain/", pm);

  bass_enhancer =
      std::make_shared<BassEnhancer>(log_tag, "com.github.wwmm.pulseeffects.bassenhancer", path + "bassenhancer/", pm);

  compressor =
      std::make_shared<Compressor>(log_tag, "com.github.wwmm.pulseeffects.compressor", path + "compressor/", pm);

  deesser = std::make_shared<Deesser>(log_tag, "com.github.wwmm.pulseeffects.deesser", path + "deesser/", pm);

  delay = std::make_shared<Delay>(log_tag, "com.github.wwmm.pulseeffects.delay", path + "delay/", pm);

  equalizer = std::make_shared<Equalizer>(log_tag, "com.github.wwmm.pulseeffects.equalizer", path + "equalizer/",
                                          "com.github.wwmm.pulseeffects.equalizer.channel",
                                          path + "equalizer/leftchannel/", path + "equalizer/rightchannel/", pm);

  exciter = std::make_shared<Exciter>(log_tag, "com.github.wwmm.pulseeffects.exciter", path + "exciter/", pm);

  filter = std::make_shared<Filter>(log_tag, "com.github.wwmm.pulseeffects.filter", path + "filter/", pm);

  gate = std::make_shared<Gate>(log_tag, "com.github.wwmm.pulseeffects.gate", path + "gate/", pm);

  limiter = std::make_shared<Limiter>(log_tag, "com.github.wwmm.pulseeffects.limiter", path + "limiter/", pm);

  maximizer = std::make_shared<Maximizer>(log_tag, "com.github.wwmm.pulseeffects.maximizer", path + "maximizer/", pm);

  multiband_compressor = std::make_shared<MultibandCompressor>(
      log_tag, "com.github.wwmm.pulseeffects.multibandcompressor", path + "multibandcompressor/", pm);

  multiband_gate = std::make_shared<MultibandGate>(log_tag, "com.github.wwmm.pulseeffects.multibandgate",
                                                   path + "multibandgate/", pm);

  output_level =
      std::make_unique<OutputLevel>(log_tag, "com.github.wwmm.pulseeffects.outputlevel", path + "outputlevel/", pm);

  reverb = std::make_shared<Reverb>(log_tag, "com.github.wwmm.pulseeffects.reverb", path + "reverb/", pm);

  rnnoise = std::make_shared<RNNoise>(log_tag, "com.github.wwmm.pulseeffects.rnnoise", path + "rnnoise/", pm);

  spectrum = std::make_unique<Spectrum>(log_tag, "com.github.wwmm.pulseeffects.spectrum",
                                        "/com/github/wwmm/pulseeffects/spectrum/", pm);

  stereo_tools =
      std::make_shared<StereoTools>(log_tag, "com.github.wwmm.pulseeffects.stereotools", path + "stereotools/", pm);

  //   convolver = std::make_shared<Convolver>(log_tag, "com.github.wwmm.pulseeffects.convolver", path + "convolver/",
  //   pm);

  //   crossfeed = std::make_shared<Crossfeed>(log_tag, "com.github.wwmm.pulseeffects.crossfeed", path + "crossfeed/",
  //   pm);

  //   crystalizer =
  //       std::make_shared<Crystalizer>(log_tag, "com.github.wwmm.pulseeffects.crystalizer", path + "crystalizer/",
  //       pm);

  //   loudness = std::make_shared<Loudness>(log_tag, "com.github.wwmm.pulseeffects.loudness", path + "loudness/", pm);

  //   pitch = std::make_shared<Pitch>(log_tag, "com.github.wwmm.pulseeffects.pitch", path + "pitch/", pm);

  plugins.insert(std::make_pair(autogain->name, autogain));
  plugins.insert(std::make_pair(bass_enhancer->name, bass_enhancer));
  plugins.insert(std::make_pair(compressor->name, compressor));
  plugins.insert(std::make_pair(deesser->name, deesser));
  plugins.insert(std::make_pair(delay->name, delay));
  plugins.insert(std::make_pair(equalizer->name, equalizer));
  plugins.insert(std::make_pair(exciter->name, exciter));
  plugins.insert(std::make_pair(filter->name, filter));
  plugins.insert(std::make_pair(gate->name, gate));
  plugins.insert(std::make_pair(limiter->name, limiter));
  plugins.insert(std::make_pair(maximizer->name, maximizer));
  plugins.insert(std::make_pair(multiband_compressor->name, multiband_compressor));
  plugins.insert(std::make_pair(multiband_gate->name, multiband_gate));
  plugins.insert(std::make_pair(reverb->name, reverb));
  plugins.insert(std::make_pair(rnnoise->name, rnnoise));
  plugins.insert(std::make_pair(stereo_tools->name, stereo_tools));
  //   plugins.insert(std::make_pair(convolver->name, convolver));
  //   plugins.insert(std::make_pair(crossfeed->name, crossfeed));
  //   plugins.insert(std::make_pair(crystalizer->name, crystalizer));
  //   plugins.insert(std::make_pair(loudness->name, loudness));
  //   plugins.insert(std::make_pair(pitch->name, pitch));
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