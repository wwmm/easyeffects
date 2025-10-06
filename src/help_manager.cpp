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
#include <qlogging.h>
#include <qobject.h>
#include <qqml.h>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include "config.h"
#include "util.hpp"

HelpManager::HelpManager(QObject* parent) : QObject(parent) {
  // NOLINTNEXTLINE(clang-analyzer-cplusplus.NewDelete)
  qmlRegisterSingletonInstance<HelpManager>("ee.help", VERSION_MAJOR, VERSION_MINOR, "Manager", this);
}

QString HelpManager::copyResourceFolder(const QString& resourcePath, const QString& tempFolderName) {
  // According to chatgpt writing to a standard temporary location should make
  // the approach work on Flatpak too. This remains to be confirmed...

  QString tempPath =
      QStandardPaths::writableLocation(QStandardPaths::TempLocation) + QDir::separator() + tempFolderName;

  if (!QDir().mkpath(tempPath)) {
    util::warning(std::format("Failed to create temp folder: {}", tempPath.toStdString()));
  }

  QDir resourceDir(resourcePath);
  for (const QString& fileName : resourceDir.entryList(QDir::Files)) {
    QString src = resourcePath + "/" + fileName;
    QString dst = tempPath + "/" + fileName;

    QFile::remove(dst);  // remove if exists

    if (!QFile::copy(src, dst)) {
      util::warning(std::format("Failed to copy {} to {}", src.toStdString(), dst.toStdString()));
    }
  }

  return tempPath;
}

void HelpManager::openManual() {
  const auto tempPath = copyResourceFolder(":/help", "easyeffects_manual");

  QString indexPath = tempPath + "/index.html";

  if (!QFile::exists(indexPath)) {
    util::warning(std::format("Index file missing: {}", indexPath.toStdString()));

    return;
  }

  QUrl url = QUrl::fromLocalFile(indexPath);

  QDesktopServices::openUrl(url);
}
