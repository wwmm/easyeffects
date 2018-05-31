#ifndef PRESETS_MANAGER_HPP
#define PRESETS_MANAGER_HPP

#include <giomm/settings.h>
#include <boost/filesystem.hpp>
#include <memory>
#include "bass_enhancer_preset.hpp"
#include "compressor_preset.hpp"
#include "crossfeed_preset.hpp"
#include "deesser_preset.hpp"
#include "delay_preset.hpp"
#include "equalizer_preset.hpp"
#include "exciter_preset.hpp"
#include "filter_preset.hpp"
#include "gate_preset.hpp"
#include "limiter_preset.hpp"
#include "maximizer_preset.hpp"
#include "panorama_preset.hpp"
#include "pitch_preset.hpp"
#include "reverb_preset.hpp"
#include "stereo_enhancer_preset.hpp"
#include "webrtc_preset.hpp"

class PresetsManager {
   public:
    PresetsManager();
    virtual ~PresetsManager();

    std::vector<std::string> get_names();
    void add(const std::string& name);
    void save(const std::string& name);
    void remove(const std::string& name);
    void load(const std::string& name);
    void import(const std::string& file_path);

   private:
    std::string log_tag = "presets_manager: ";

    boost::filesystem::path presets_dir;

    Glib::RefPtr<Gio::Settings> sie_settings, soe_settings;

    std::unique_ptr<LimiterPreset> limiter;
    std::unique_ptr<BassEnhancerPreset> bass_enhancer;
    std::unique_ptr<CompressorPreset> compressor;
    std::unique_ptr<CrossfeedPreset> crossfeed;
    std::unique_ptr<DeesserPreset> deesser;
    std::unique_ptr<DelayPreset> delay;
    std::unique_ptr<EqualizerPreset> equalizer;
    std::unique_ptr<ExciterPreset> exciter;
    std::unique_ptr<FilterPreset> filter;
    std::unique_ptr<GatePreset> gate;
    std::unique_ptr<MaximizerPreset> maximizer;
    std::unique_ptr<PanoramaPreset> panorama;
    std::unique_ptr<PitchPreset> pitch;
    std::unique_ptr<ReverbPreset> reverb;
    std::unique_ptr<StereoEnhancerPreset> stereo_enhancer;
    std::unique_ptr<WebrtcPreset> webrtc;
};

#endif
