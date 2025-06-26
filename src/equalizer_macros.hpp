#pragma once

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_BAND_PORT(settings_obj, key, getter, setter, onChangedSignal)                                           \
  {                                                                                                                  \
    lv2_wrapper->set_control_port_value(key, static_cast<float>(settings_obj->getter()));                            \
    lv2_wrapper->sync_funcs.emplace_back([&]() { settings_obj->setter(lv2_wrapper->get_control_port_value(key)); }); \
    connect(settings_obj, &onChangedSignal, [this]() {                                                               \
      if (this == nullptr || settings_obj == nullptr || lv2_wrapper == nullptr) {                                    \
        return;                                                                                                      \
      }                                                                                                              \
      lv2_wrapper->set_control_port_value(key, static_cast<float>(settings_obj->getter()));                          \
    });                                                                                                              \
  }

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_BANDS_TYPE(settings_obj)                                                                                 \
  {                                                                                                                   \
    using namespace tags::equalizer;                                                                                  \
    BIND_BAND_PORT(settings_obj, ftl[0].data(), band0Type, setBand0Type, db::EqualizerChannel::band0TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[1].data(), band1Type, setBand1Type, db::EqualizerChannel::band1TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[2].data(), band2Type, setBand2Type, db::EqualizerChannel::band2TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[3].data(), band3Type, setBand3Type, db::EqualizerChannel::band3TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[4].data(), band4Type, setBand4Type, db::EqualizerChannel::band4TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[5].data(), band5Type, setBand5Type, db::EqualizerChannel::band5TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[6].data(), band6Type, setBand6Type, db::EqualizerChannel::band6TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[7].data(), band7Type, setBand7Type, db::EqualizerChannel::band7TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[8].data(), band8Type, setBand8Type, db::EqualizerChannel::band8TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[9].data(), band9Type, setBand9Type, db::EqualizerChannel::band9TypeChanged);     \
    BIND_BAND_PORT(settings_obj, ftl[10].data(), band10Type, setBand10Type, db::EqualizerChannel::band10TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[11].data(), band11Type, setBand11Type, db::EqualizerChannel::band11TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[12].data(), band12Type, setBand12Type, db::EqualizerChannel::band12TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[13].data(), band13Type, setBand13Type, db::EqualizerChannel::band13TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[14].data(), band14Type, setBand14Type, db::EqualizerChannel::band14TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[15].data(), band15Type, setBand15Type, db::EqualizerChannel::band15TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[16].data(), band16Type, setBand16Type, db::EqualizerChannel::band16TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[17].data(), band17Type, setBand17Type, db::EqualizerChannel::band17TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[18].data(), band18Type, setBand18Type, db::EqualizerChannel::band18TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[19].data(), band19Type, setBand19Type, db::EqualizerChannel::band19TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[20].data(), band20Type, setBand20Type, db::EqualizerChannel::band20TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[21].data(), band21Type, setBand21Type, db::EqualizerChannel::band21TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[22].data(), band22Type, setBand22Type, db::EqualizerChannel::band22TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[23].data(), band23Type, setBand23Type, db::EqualizerChannel::band23TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[24].data(), band24Type, setBand24Type, db::EqualizerChannel::band24TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[25].data(), band25Type, setBand25Type, db::EqualizerChannel::band25TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[26].data(), band26Type, setBand26Type, db::EqualizerChannel::band26TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[27].data(), band27Type, setBand27Type, db::EqualizerChannel::band27TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[28].data(), band28Type, setBand28Type, db::EqualizerChannel::band28TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[29].data(), band29Type, setBand29Type, db::EqualizerChannel::band29TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[30].data(), band30Type, setBand30Type, db::EqualizerChannel::band30TypeChanged); \
    BIND_BAND_PORT(settings_obj, ftl[31].data(), band31Type, setBand31Type, db::EqualizerChannel::band31TypeChanged); \
  }

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_BANDS_MODE(settings_obj)                                                                                 \
  {                                                                                                                   \
    using namespace tags::equalizer;                                                                                  \
    BIND_BAND_PORT(settings_obj, fml[0].data(), band0Mode, setBand0Mode, db::EqualizerChannel::band0ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[1].data(), band1Mode, setBand1Mode, db::EqualizerChannel::band1ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[2].data(), band2Mode, setBand2Mode, db::EqualizerChannel::band2ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[3].data(), band3Mode, setBand3Mode, db::EqualizerChannel::band3ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[4].data(), band4Mode, setBand4Mode, db::EqualizerChannel::band4ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[5].data(), band5Mode, setBand5Mode, db::EqualizerChannel::band5ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[6].data(), band6Mode, setBand6Mode, db::EqualizerChannel::band6ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[7].data(), band7Mode, setBand7Mode, db::EqualizerChannel::band7ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[8].data(), band8Mode, setBand8Mode, db::EqualizerChannel::band8ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[9].data(), band9Mode, setBand9Mode, db::EqualizerChannel::band9ModeChanged);     \
    BIND_BAND_PORT(settings_obj, fml[10].data(), band10Mode, setBand10Mode, db::EqualizerChannel::band10ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[11].data(), band11Mode, setBand11Mode, db::EqualizerChannel::band11ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[12].data(), band12Mode, setBand12Mode, db::EqualizerChannel::band12ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[13].data(), band13Mode, setBand13Mode, db::EqualizerChannel::band13ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[14].data(), band14Mode, setBand14Mode, db::EqualizerChannel::band14ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[15].data(), band15Mode, setBand15Mode, db::EqualizerChannel::band15ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[16].data(), band16Mode, setBand16Mode, db::EqualizerChannel::band16ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[17].data(), band17Mode, setBand17Mode, db::EqualizerChannel::band17ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[18].data(), band18Mode, setBand18Mode, db::EqualizerChannel::band18ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[19].data(), band19Mode, setBand19Mode, db::EqualizerChannel::band19ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[20].data(), band20Mode, setBand20Mode, db::EqualizerChannel::band20ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[21].data(), band21Mode, setBand21Mode, db::EqualizerChannel::band21ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[22].data(), band22Mode, setBand22Mode, db::EqualizerChannel::band22ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[23].data(), band23Mode, setBand23Mode, db::EqualizerChannel::band23ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[24].data(), band24Mode, setBand24Mode, db::EqualizerChannel::band24ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[25].data(), band25Mode, setBand25Mode, db::EqualizerChannel::band25ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[26].data(), band26Mode, setBand26Mode, db::EqualizerChannel::band26ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[27].data(), band27Mode, setBand27Mode, db::EqualizerChannel::band27ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[28].data(), band28Mode, setBand28Mode, db::EqualizerChannel::band28ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[29].data(), band29Mode, setBand29Mode, db::EqualizerChannel::band29ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[30].data(), band30Mode, setBand30Mode, db::EqualizerChannel::band30ModeChanged); \
    BIND_BAND_PORT(settings_obj, fml[31].data(), band31Mode, setBand31Mode, db::EqualizerChannel::band31ModeChanged); \
  }

// NOLINTBEGIN(bugprone-macro-parentheses,cppcoreguidelines-macro-usage)
#define BIND_BANDS_SLOPE(settings_obj)                                                                              \
  {                                                                                                                 \
    using namespace tags::equalizer;                                                                                \
    BIND_BAND_PORT(settings_obj, sl[0].data(), band0Slope, setBand0Slope, db::EqualizerChannel::band0SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[1].data(), band1Slope, setBand1Slope, db::EqualizerChannel::band1SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[2].data(), band2Slope, setBand2Slope, db::EqualizerChannel::band2SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[3].data(), band3Slope, setBand3Slope, db::EqualizerChannel::band3SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[4].data(), band4Slope, setBand4Slope, db::EqualizerChannel::band4SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[5].data(), band5Slope, setBand5Slope, db::EqualizerChannel::band5SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[6].data(), band6Slope, setBand6Slope, db::EqualizerChannel::band6SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[7].data(), band7Slope, setBand7Slope, db::EqualizerChannel::band7SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[8].data(), band8Slope, setBand8Slope, db::EqualizerChannel::band8SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[9].data(), band9Slope, setBand9Slope, db::EqualizerChannel::band9SlopeChanged); \
    BIND_BAND_PORT(settings_obj, sl[10].data(), band10Slope, setBand10Slope,                                        \
                   db::EqualizerChannel::band10SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[11].data(), band11Slope, setBand11Slope,                                        \
                   db::EqualizerChannel::band11SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[12].data(), band12Slope, setBand12Slope,                                        \
                   db::EqualizerChannel::band12SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[13].data(), band13Slope, setBand13Slope,                                        \
                   db::EqualizerChannel::band13SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[14].data(), band14Slope, setBand14Slope,                                        \
                   db::EqualizerChannel::band14SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[15].data(), band15Slope, setBand15Slope,                                        \
                   db::EqualizerChannel::band15SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[16].data(), band16Slope, setBand16Slope,                                        \
                   db::EqualizerChannel::band16SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[17].data(), band17Slope, setBand17Slope,                                        \
                   db::EqualizerChannel::band17SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[18].data(), band18Slope, setBand18Slope,                                        \
                   db::EqualizerChannel::band18SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[19].data(), band19Slope, setBand19Slope,                                        \
                   db::EqualizerChannel::band19SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[20].data(), band20Slope, setBand20Slope,                                        \
                   db::EqualizerChannel::band20SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[21].data(), band21Slope, setBand21Slope,                                        \
                   db::EqualizerChannel::band21SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[22].data(), band22Slope, setBand22Slope,                                        \
                   db::EqualizerChannel::band22SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[23].data(), band23Slope, setBand23Slope,                                        \
                   db::EqualizerChannel::band23SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[24].data(), band24Slope, setBand24Slope,                                        \
                   db::EqualizerChannel::band24SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[25].data(), band25Slope, setBand25Slope,                                        \
                   db::EqualizerChannel::band25SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[26].data(), band26Slope, setBand26Slope,                                        \
                   db::EqualizerChannel::band26SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[27].data(), band27Slope, setBand27Slope,                                        \
                   db::EqualizerChannel::band27SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[28].data(), band28Slope, setBand28Slope,                                        \
                   db::EqualizerChannel::band28SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[29].data(), band29Slope, setBand29Slope,                                        \
                   db::EqualizerChannel::band29SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[30].data(), band30Slope, setBand30Slope,                                        \
                   db::EqualizerChannel::band30SlopeChanged);                                                       \
    BIND_BAND_PORT(settings_obj, sl[31].data(), band31Slope, setBand31Slope,                                        \
                   db::EqualizerChannel::band31SlopeChanged);                                                       \
  }
