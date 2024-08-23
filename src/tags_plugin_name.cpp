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
  modelMap = {{BaseName::autogain, i18n("Autogain")},
              {BaseName::bass_enhancer, i18n("Bass Enhancer")},
              {BaseName::bass_loudness, i18n("Bass Loudness")},
              {BaseName::compressor, i18n("Compressor")},
              {BaseName::convolver, i18n("Convolver")},
              {BaseName::crossfeed, i18n("Crossfeed")},
              {BaseName::crystalizer, i18n("Crystalizer")},
              {BaseName::deepfilternet, i18n("Deep Noise Remover")},
              {BaseName::deesser, i18n("Deesser")},
              {BaseName::delay, i18n("Delay")},
              {BaseName::echo_canceller, i18n("Echo Canceller")},
              {BaseName::equalizer, i18n("Equalizer")},
              {BaseName::exciter, i18n("Exciter")},
              {BaseName::expander, i18n("Expander")},
              {BaseName::filter, i18n("Filter")},
              {BaseName::gate, i18n("Gate")},
              {BaseName::level_meter, i18n("Level Meter")},
              {BaseName::limiter, i18n("Limiter")},
              {BaseName::loudness, i18n("Loudness")},
              {BaseName::maximizer, i18n("Maximizer")},
              {BaseName::multiband_compressor, i18n("Multiband Compressor")},
              {BaseName::multiband_gate, i18n("Multiband Gate")},
              {BaseName::pitch, i18n("Pitch")},
              {BaseName::reverb, i18n("Reverberation")},
              {BaseName::rnnoise, i18n("Noise Reduction")},
              {BaseName::speex, i18n("Speech Processor")},
              {BaseName::stereo_tools, i18n("Stereo Tools")}};

  auto* proxyModel = new QSortFilterProxyModel(this);

  proxyModel->setSourceModel(this);
  proxyModel->setFilterRole(Model::Roles::Name);
  proxyModel->setSortRole(Model::Roles::Name);
  proxyModel->setDynamicSortFilter(true);
  proxyModel->sort(0);

  qmlRegisterSingletonInstance<BaseName>("EEtagsPluginName", VERSION_MAJOR, VERSION_MINOR, "BasePluginName",
                                         &base_name);

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