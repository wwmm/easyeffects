#include "tags_plugin_name.hpp"
#include <qabstractitemmodel.h>
#include <qhash.h>
#include <qhashfunctions.h>
#include <qobject.h>
#include <qqml.h>
#include <qsortfilterproxymodel.h>
#include <qstringview.h>
#include <qvariant.h>
#include <KLocalizedString>
#include <iterator>
#include "config.h"

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

  qmlRegisterSingletonInstance<QSortFilterProxyModel>("EEtagsPluginName", VERSION_MAJOR, VERSION_MINOR,
                                                      "PluginsNameModel", proxyModel);
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

}  // namespace tags::plugin_name