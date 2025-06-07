/**
 * Copyright (C) 2021-2024 Saturneric <eric@bktus.com>
 *
 * This file is part of GpgFrontend.
 *
 * GpgFrontend is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GpgFrontend is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GpgFrontend. If not, see <https://www.gnu.org/licenses/>.
 *
 * The initial version of the source code is inherited from
 * the gpg4usb project, which is under GPL-3.0-or-later.
 *
 * All the source code of GpgFrontend was modified and released by
 * Saturneric <eric@bktus.com> starting on May 12, 2021.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "GlobalSettingStation.h"

#include "GpgFrontendBuildInstallInfo.h"

//
#include "core/function/GFBufferFactory.h"
#include "core/function/PassphraseGenerator.h"
#include "core/module/ModuleManager.h"
#include "core/utils/FilesystemUtils.h"

namespace GpgFrontend {

class GlobalSettingStation::Impl {
 public:
  /**
   * @brief Construct a new Global Setting Station object
   *
   */
  explicit Impl() noexcept {
    LOG_I() << "app path: " << app_path_;
    LOG_I() << "app working path: " << working_path_;

    auto portable_file_path = working_path_ + "/PORTABLE.txt";
    if (QFileInfo(portable_file_path).exists()) {
      Module::UpsertRTValue("core", "env.state.portable", 1);
      LOG_I() << "GpgFrontend runs in the portable mode now";

#if defined(__linux__)
      if (IsAppImageENV()) {
        LOG_I() << "app image path: " << qEnvironmentVariable("APPIMAGE");
        QFileInfo info(
            QString::fromUtf8(qEnvironmentVariable("APPIMAGE").toUtf8()));
        app_path_ = info.canonicalPath();
      }
#endif

      app_data_path_ = QDir(app_path_ + "/../").canonicalPath();
      app_config_path_ = app_data_path_ + "/config";

      portable_mode_ = true;
    }

    LOG_I() << "app data path: " << app_data_path_;
    LOG_I() << "app secure path: " << app_secure_path();
    LOG_I() << "app log path: " << app_log_path();
    LOG_I() << "app modules path: " << app_mods_path();

#if defined(_WIN32) || defined(WIN32)
    LOG_I() << "app config path: " << app_config_path_;
    if (!QDir(app_config_path_).exists()) QDir(app_config_path_).mkpath(".");
#else
    if (IsProtableMode()) {
      LOG_I() << "app config path: " << app_config_path_;
      if (!QDir(app_config_path_).exists()) QDir(app_config_path_).mkpath(".");
    }
#endif

    if (!QDir(app_data_path_).exists()) QDir(app_data_path_).mkpath(".");
    if (!QDir(app_log_path()).exists()) QDir(app_log_path()).mkpath(".");
    if (!QDir(app_secure_path()).exists()) QDir(app_log_path()).mkpath(".");
    if (!QDir(app_mods_path()).exists()) QDir(app_mods_path()).mkpath(".");
    if (!QDir(app_data_objs_path()).exists()) {
      QDir(app_data_objs_path()).mkpath(".");
    }

    // generate or fetch app secure key
    init_app_secure_key();
  }

  [[nodiscard]] auto GetSettings() -> QSettings {
#if defined(_WIN32) || defined(WIN32)
    return QSettings(app_config_file_path(), QSettings::IniFormat);
#else
    if (IsProtableMode()) return {app_config_file_path(), QSettings::IniFormat};
    return QSettings();
#endif
  }

  [[nodiscard]] auto GetLogFilesSize() const -> QString {
    return GetHumanFriendlyFileSize(GetFileSizeByPath(app_log_path(), "*.log"));
  }

  [[nodiscard]] auto GetDataObjectsFilesSize() const -> QString {
    return GetHumanFriendlyFileSize(
        GetFileSizeByPath(app_data_objs_path(), "*"));
  }

  void ClearAllLogFiles() const {
    DeleteAllFilesByPattern(app_log_path(), "*.log");
  }

  void ClearAllDataObjects() const {
    DeleteAllFilesByPattern(app_data_objs_path(), "*");
  }

  /**
   * @brief Get the App Dir object
   *
   * @return QString
   */
  [[nodiscard]] auto GetAppDir() const -> QString { return app_path_; }

  /**
   * @brief Get the App Data Path object
   *
   * @return QString
   */
  [[nodiscard]] auto GetAppDataPath() const -> QString {
    return app_data_path_;
  }

  /**
   * @brief Get the Log Dir object
   *
   * @return QString
   */
  [[nodiscard]] auto GetLogDir() const -> QString { return app_log_path(); }

  /**
   * @brief Get the Config Path object
   *
   * @return QString
   */
  [[nodiscard]] auto GetConfigPath() const -> QString {
    return app_config_file_path();
  }

  /**
   * @brief Get the Modules Dir object
   *
   * @return QString
   */
  [[nodiscard]] auto GetModulesDir() const -> QString {
    return app_mods_path();
  }

  [[nodiscard]] auto GetIntegratedModulePath() const -> QString {
    const auto exec_binary_path = GetAppDir();

#if defined(__linux__)
    // AppImage
    if (IsAppImageENV()) {
      return qEnvironmentVariable("APPDIR") + "/usr/lib/modules";
    }
    // Flatpak
    if (IsFlatpakENV()) {
      return "/app/lib/gpgfrontend/modules";
    }
#endif

#if defined(_WIN32) || defined(WIN32)
    return exec_binary_path + "/../modules";
#endif

#if defined(__APPLE__) && defined(__MACH__)

#ifdef NDEBUG
    return exec_binary_path + "/../PlugIns";
#endif

#endif

    // Package or Install
    auto module_install_path = QString(APP_LIB_PATH) + "/gpgfrontend/modules";
    if (QFileInfo(module_install_path).exists()) {
      return module_install_path;
    }

    return exec_binary_path + "/modules";
  }

  [[nodiscard]] auto IsProtableMode() const -> bool { return portable_mode_; }

  auto GetAppSecureKey() -> GFBuffer { return app_secure_key_; }

  [[nodiscard]] auto GetDataObjectsPath() const -> QString {
    return app_data_objs_path();
  }

 private:
  [[nodiscard]] auto app_config_file_path() const -> QString {
    return app_config_path_ + "/config.ini";
  }

  [[nodiscard]] auto app_data_objs_path() const -> QString {
    return app_data_path_ + "/data_objs";
  }

  [[nodiscard]] auto app_log_path() const -> QString {
    return app_data_path_ + "/logs";
  }

  [[nodiscard]] auto app_mods_path() const -> QString {
    return app_data_path_ + "/mods";
  }

  [[nodiscard]] auto app_secure_path() const -> QString {
    return app_data_path_ + "/secure";
  }

  [[nodiscard]] auto app_secure_key_path() const -> QString {
    return app_secure_path() + "/app.key";
  }

  void init_app_secure_key() {
    if (!QFileInfo(app_secure_key_path()).exists()) {
      auto key = PassphraseGenerator::GenerateBytesByOpenSSL(256);
      if (!key) {
        LOG_E() << "generate app secure key failed";
        return;
      }

      if (!GFBufferFactory::ToFile(app_secure_key_path(), *key)) {
        LOG_E() << "write app secure key failed: " << app_secure_key_path();
      }

      app_secure_key_ = *key;
    } else {
      auto key = GFBufferFactory::FromFile(app_secure_key_path());
      if (!key) {
        LOG_E() << "write app secure key failed: " << app_secure_key_path();
      }

      app_secure_key_ = *key;
    }
  }

  bool portable_mode_ = false;
  QString app_path_ = QCoreApplication::applicationDirPath();
  QString working_path_ = QDir::currentPath();
  QString app_data_path_ = QString{
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)};
  QString app_config_path_ = QString{
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
  GFBuffer app_secure_key_;
};

GlobalSettingStation::GlobalSettingStation(int channel) noexcept
    : SingletonFunctionObject<GlobalSettingStation>(channel),
      p_(SecureCreateUniqueObject<Impl>()) {}

GlobalSettingStation::~GlobalSettingStation() noexcept = default;

auto GlobalSettingStation::GetSettings() const -> QSettings {
  return p_->GetSettings();
}

auto GlobalSettingStation::GetAppDir() const -> QString {
  return p_->GetAppDir();
}

auto GlobalSettingStation::GetAppDataPath() const -> QString {
  return p_->GetAppDataPath();
}

[[nodiscard]] auto GlobalSettingStation::GetAppLogPath() const -> QString {
  return p_->GetLogDir();
}

[[nodiscard]] auto GlobalSettingStation::GetModulesDir() const -> QString {
  return p_->GetModulesDir();
}

auto GlobalSettingStation::GetLogFilesSize() const -> QString {
  return p_->GetLogFilesSize();
}

auto GlobalSettingStation::GetDataObjectsFilesSize() const -> QString {
  return p_->GetDataObjectsFilesSize();
}

void GlobalSettingStation::ClearAllLogFiles() const { p_->ClearAllLogFiles(); }

void GlobalSettingStation::ClearAllDataObjects() const {
  p_->ClearAllDataObjects();
}

auto GlobalSettingStation::GetConfigPath() const -> QString {
  return p_->GetConfigPath();
}

auto GlobalSettingStation::GetIntegratedModulePath() const -> QString {
  return p_->GetIntegratedModulePath();
}

auto GlobalSettingStation::IsProtableMode() const -> bool {
  return p_->IsProtableMode();
}
auto GetSettings() -> QSettings {
  return GlobalSettingStation::GetInstance().GetSettings();
}

auto GlobalSettingStation::GetAppSecureKey() -> GFBuffer {
  return p_->GetAppSecureKey();
}

auto GlobalSettingStation::GetDataObjectsDir() const -> QString {
  return p_->GetDataObjectsPath();
}
}  // namespace GpgFrontend