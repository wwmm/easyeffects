/**
 * Copyright © 2017-2025 Wellington Wallace
 *
 * This file is part of Easy Effects.
 *
 * Easy Effects is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Easy Effects is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Easy Effects. If not, see <https://www.gnu.org/licenses/>.
 */

#include "help_manager.hpp"
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <qobject.h>
#include <qqml.h>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <format>
#include "config.h"
#include "util.hpp"

HelpManager::HelpManager(QObject* parent) : QObject(parent) {
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<HelpManager>("ee.help", VERSION_MAJOR, VERSION_MINOR, "Manager", this);
}

QString HelpManager::copyResourceFolder(const QString& resourcePath, const QString& dataFolderName) {
  QString dataPath =
      QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QDir::separator() + dataFolderName;

  if (!QDir().mkpath(dataPath)) {
    util::warning(std::format("Failed to create temp folder: {}", dataPath.toStdString()));
  }

  QDir resourceDir(resourcePath);
  for (const QString& fileName : resourceDir.entryList(QDir::Files)) {
    QString src = resourcePath + "/" + fileName;
    QString dst = dataPath + "/" + fileName;

    QFile::remove(dst);  // remove if exists

    if (!QFile::copy(src, dst)) {
      util::warning(std::format("Failed to copy {} to {}", src.toStdString(), dst.toStdString()));
    }
  }

  return dataPath;
}

void HelpManager::openManual() {
  const auto dataPath = copyResourceFolder(":/help", "easyeffects_manual");

  QString indexPath = dataPath + "/index.html";

  if (!QFile::exists(indexPath)) {
    util::warning(std::format("Index file missing: {}", indexPath.toStdString()));

    return;
  }

  QUrl url = QUrl::fromLocalFile(indexPath);

  if (QDesktopServices::openUrl(url)) {
    return;
  }

  util::warning(" We failed to launch a browser to open the manual index.html file. We will try xdg-open as fallback.");

  QStringList arguments;

  arguments << indexPath;

  if (!QProcess::startDetached("xdg-open", arguments)) {
    util::warning("We failed to launch xdg-open. The manual can be opened.");
  }
}
