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
#include "config.h"
#include "util.hpp"

namespace {

const auto id_regex = std::regex(R"(#(\d+)$)");

}

namespace tags::plugin_name {

Model::Model(QObject* parent) : QAbstractListModel(parent) {
  modelMap = {{PluginName::autogain, i18n("Autogain")},
              {PluginName::bass_enhancer, i18n("Bass Enhancer")},
              {PluginName::bass_loudness, i18n("Bass Loudness")},
              {PluginName::compressor, i18n("Compressor")},
              {PluginName::convolver, i18n("Convolver")},
              {PluginName::crossfeed, i18n("Crossfeed")},
              {PluginName::crystalizer, i18n("Crystalizer")},
              {PluginName::deepfilternet, i18n("Deep Noise Remover")},
              {PluginName::deesser, i18n("Deesser")},
              {PluginName::delay, i18n("Delay")},
              {PluginName::echo_canceller, i18n("Echo Canceller")},
              {PluginName::equalizer, i18n("Equalizer")},
              {PluginName::exciter, i18n("Exciter")},
              {PluginName::expander, i18n("Expander")},
              {PluginName::filter, i18n("Filter")},
              {PluginName::gate, i18n("Gate")},
              {PluginName::level_meter, i18n("Level Meter")},
              {PluginName::limiter, i18n("Limiter")},
              {PluginName::loudness, i18n("Loudness")},
              {PluginName::maximizer, i18n("Maximizer")},
              {PluginName::multiband_compressor, i18n("Multiband Compressor")},
              {PluginName::multiband_gate, i18n("Multiband Gate")},
              {PluginName::pitch, i18n("Pitch")},
              {PluginName::reverb, i18n("Reverberation")},
              {PluginName::rnnoise, i18n("Noise Reduction")},
              {PluginName::speex, i18n("Speech Processor")},
              {PluginName::stereo_tools, i18n("Stereo Tools")}};

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