#include "tags_plugin_name.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qlist.h>
#include <qobject.h>
#include <qqml.h>
#include <qsortfilterproxymodel.h>
#include <qstringview.h>
#include <qtypes.h>
#include <qvariant.h>
#include <KLocalizedString>
#include <iterator>
#include <regex>
#include <string>
#include <string_view>
#include "config.h"
#include "util.hpp"

namespace {

const auto id_regex = std::regex(R"(#(\d+)$)");

}

namespace tags::plugin_name {

Model::Model(QObject* parent) : QAbstractListModel(parent) {
  modelMap = {{autogain, i18n("Autogain")},
              {bass_enhancer, i18n("Bass Enhancer")},
              {bass_loudness, i18n("Bass Loudness")},
              {compressor, i18n("Compressor")},
              {convolver, i18n("Convolver")},
              {crossfeed, i18n("Crossfeed")},
              {crystalizer, i18n("Crystalizer")},
              {deepfilternet, i18n("Deep Noise Remover")},
              {deesser, i18n("Deesser")},
              {delay, i18n("Delay")},
              {echo_canceller, i18n("Echo Canceller")},
              {equalizer, i18n("Equalizer")},
              {exciter, i18n("Exciter")},
              {expander, i18n("Expander")},
              {filter, i18n("Filter")},
              {gate, i18n("Gate")},
              {level_meter, i18n("Level Meter")},
              {limiter, i18n("Limiter")},
              {loudness, i18n("Loudness")},
              {maximizer, i18n("Maximizer")},
              {multiband_compressor, i18n("Multiband Compressor")},
              {multiband_gate, i18n("Multiband Gate")},
              {pitch, i18n("Pitch")},
              {reverb, i18n("Reverberation")},
              {rnnoise, i18n("Noise Reduction")},
              {speex, i18n("Speech Processor")},
              {stereo_tools, i18n("Stereo Tools")}};

  auto* proxyModel = new QSortFilterProxyModel(this);

  proxyModel->setSourceModel(this);
  proxyModel->setFilterRole(Model::Roles::Name);
  proxyModel->setSortRole(Model::Roles::Name);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->sort(0);

  qmlRegisterSingletonInstance<Model>("EEtagsPluginName", VERSION_MAJOR, VERSION_MINOR, "PluginsNameModel", this);

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("EEtagsPluginName", VERSION_MAJOR, VERSION_MINOR,
                                                      "SortedPluginsNameModel", proxyModel);
}

int Model::rowCount(const QModelIndex& /*parent*/) const {
  return modelMap.size();
}

QHash<int, QByteArray> Model::roleNames() const {
  return {{Roles::Name, "name"}, {Roles::TranslatedName, "translatedName"}};
}

QVariant Model::data(const QModelIndex& index, int role) const {
  if (modelMap.empty()) {
    return "";
  }

  const auto it = std::next(modelMap.begin(), index.row());

  switch (role) {
    case Roles::Name:
      return it.key();
    case Roles::TranslatedName:
      return it.value();
    default:
      return {};
  }
}

auto Model::getMap() -> QMap<QString, QString> {
  return modelMap;
}

QString Model::translate(const QString& baseName) {
  return modelMap[baseName];
}

QList<QString> Model::getBaseNames() {
  return modelMap.keys();
}

auto get_base_name(std::string_view name) -> std::string {
  if (name.starts_with(tags::plugin_name::autogain)) {
    return tags::plugin_name::autogain;
  }

  if (name.starts_with(tags::plugin_name::bass_enhancer)) {
    return tags::plugin_name::bass_enhancer;
  }

  if (name.starts_with(tags::plugin_name::bass_loudness)) {
    return tags::plugin_name::bass_loudness;
  }

  if (name.starts_with(tags::plugin_name::compressor)) {
    return tags::plugin_name::compressor;
  }

  if (name.starts_with(tags::plugin_name::convolver)) {
    return tags::plugin_name::convolver;
  }

  if (name.starts_with(tags::plugin_name::crossfeed)) {
    return tags::plugin_name::crossfeed;
  }

  if (name.starts_with(tags::plugin_name::crystalizer)) {
    return tags::plugin_name::crystalizer;
  }

  if (name.starts_with(tags::plugin_name::deepfilternet)) {
    return tags::plugin_name::deepfilternet;
  }

  if (name.starts_with(tags::plugin_name::deesser)) {
    return tags::plugin_name::deesser;
  }

  if (name.starts_with(tags::plugin_name::delay)) {
    return tags::plugin_name::delay;
  }

  if (name.starts_with(tags::plugin_name::echo_canceller)) {
    return tags::plugin_name::echo_canceller;
  }

  if (name.starts_with(tags::plugin_name::equalizer)) {
    return tags::plugin_name::equalizer;
  }

  if (name.starts_with(tags::plugin_name::exciter)) {
    return tags::plugin_name::exciter;
  }

  if (name.starts_with(tags::plugin_name::expander)) {
    return tags::plugin_name::expander;
  }

  if (name.starts_with(tags::plugin_name::filter)) {
    return tags::plugin_name::filter;
  }

  if (name.starts_with(tags::plugin_name::gate)) {
    return tags::plugin_name::gate;
  }

  if (name.starts_with(tags::plugin_name::level_meter)) {
    return tags::plugin_name::level_meter;
  }

  if (name.starts_with(tags::plugin_name::limiter)) {
    return tags::plugin_name::limiter;
  }

  if (name.starts_with(tags::plugin_name::loudness)) {
    return tags::plugin_name::loudness;
  }

  if (name.starts_with(tags::plugin_name::maximizer)) {
    return tags::plugin_name::maximizer;
  }

  if (name.starts_with(tags::plugin_name::multiband_compressor)) {
    return tags::plugin_name::multiband_compressor;
  }

  if (name.starts_with(tags::plugin_name::multiband_gate)) {
    return tags::plugin_name::multiband_gate;
  }

  if (name.starts_with(tags::plugin_name::pitch)) {
    return tags::plugin_name::pitch;
  }

  if (name.starts_with(tags::plugin_name::reverb)) {
    return tags::plugin_name::reverb;
  }

  if (name.starts_with(tags::plugin_name::rnnoise)) {
    return tags::plugin_name::rnnoise;
  }

  if (name.starts_with(tags::plugin_name::speex)) {
    return tags::plugin_name::speex;
  }

  if (name.starts_with(tags::plugin_name::stereo_tools)) {
    return tags::plugin_name::stereo_tools;
  }

  return "";
}

auto get_id(const std::string& name) -> uint {
  std::smatch matches;

  std::regex_search(name, matches, id_regex);

  if (matches.size() != 2U) {
    return 0U;
  }

  if (uint id = 0U; util::str_to_num(matches[1], id)) {
    return id;
  }

  return 0U;
}

}  // namespace tags::plugin_name