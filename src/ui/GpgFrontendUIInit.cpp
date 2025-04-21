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

#include "GpgFrontendUIInit.h"

#include <QtNetwork>

#include "core/GpgConstants.h"
#include "core/function/CoreSignalStation.h"
#include "core/function/GlobalSettingStation.h"
#include "core/model/GpgPassphraseContext.h"
#include "core/module/ModuleManager.h"
#include "ui/UIModuleManager.h"
#include "ui/UISignalStation.h"
#include "ui/UserInterfaceUtils.h"
#include "ui/main_window/MainWindow.h"

namespace GpgFrontend::UI {

QContainer<QTranslator*> registered_translators;
QContainer<QByteArray> loaded_qm_datum;

extern void InitUITranslations();

void WaitEnvCheckingProcess() {
  FLOG_D("need to wait for env checking process");

  // create and show loading window before starting the main window
  auto* progress_dialog = new QProgressDialog();
  progress_dialog->setMaximum(0);
  progress_dialog->setMinimum(0);
  auto* waiting_dialog_label = new QLabel(
      QCoreApplication::tr("Loading Gnupg Info...") + "<br /><br />" +
      QCoreApplication::tr(
          "If this process is too slow, please set the key "
          "server address appropriately in the gnupg configuration "
          "file (depending "
          "on the network situation in your country or region)."));
  waiting_dialog_label->setWordWrap(true);
  progress_dialog->setLabel(waiting_dialog_label);
  progress_dialog->resize(420, 120);
  QApplication::connect(CoreSignalStation::GetInstance(),
                        &CoreSignalStation::SignalCoreFullyLoaded,
                        progress_dialog, [=]() {
                          LOG_D() << "ui caught signal: core fully loaded";
                          progress_dialog->finished(0);
                          progress_dialog->deleteLater();
                        });

  QApplication::connect(CoreSignalStation::GetInstance(),
                        &CoreSignalStation::SignalBadGnupgEnv, progress_dialog,
                        [=]() {
                          LOG_D() << "ui caught signal: core loading failed";
                          progress_dialog->finished(0);
                          progress_dialog->deleteLater();
                        });

  // new local event looper
  QEventLoop looper;
  QApplication::connect(CoreSignalStation::GetInstance(),
                        &CoreSignalStation::SignalCoreFullyLoaded, &looper,
                        &QEventLoop::quit);

  QApplication::connect(progress_dialog, &QProgressDialog::canceled, [=]() {
    FLOG_D("cancel clicked on waiting dialog");
    QApplication::quit();
    exit(0);
  });

  auto env_state =
      Module::RetrieveRTValueTypedOrDefault<>("core", "env.state.all", 0);
  FLOG_D("ui is ready to wait for env initialized, env_state: %d", env_state);

  // check twice to avoid some unlucky sitations
  if (env_state == 1) {
    FLOG_D("env state turned initialized before the looper start");
    progress_dialog->finished(0);
    progress_dialog->deleteLater();
    return;
  }

  // show the loading window
  progress_dialog->setModal(true);
  progress_dialog->setFocus();
  progress_dialog->show();

  // block the main thread until the gpg context is loaded
  looper.exec();
}

void PreInitGpgFrontendUI() {
  CommonUtils::GetInstance();

  // declare module ui entry mount points
  UIModuleManager::GetInstance().DeclareMountPoint("AboutDialogTabs", "QWidget",
                                                   {});
  UIModuleManager::GetInstance().DeclareMountPoint("GnuPGControllerDialogTabs",
                                                   "QWidget", {});
}

void InitGpgFrontendUI(QApplication* app) {
  // init locale
  InitUITranslations();

  auto settings = GetSettings();
  auto theme = settings.value("appearance/theme").toString();

  // Set Fusion style for better dark mode support across platforms
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  // Check if system is using dark mode by comparing text/background lightness
  QPalette systemPalette = QApplication::palette();
  QColor windowColor = systemPalette.color(QPalette::Window);
  QColor textColor = systemPalette.color(QPalette::WindowText);
  
  // In dark themes, text is typically lighter than the background
  bool isDarkMode = textColor.lightness() > windowColor.lightness();
  
  FLOG_D("Dark mode detected: %s", isDarkMode ? "true" : "false");

  if (isDarkMode) {
    FLOG_D("Applying dark palette...");
    // Apply dark palette for Fusion
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    // Apply the dark palette
    QApplication::setPalette(darkPalette);
  }

  // If user has explicitly set a theme in settings, use that instead
  auto available_styles = QStyleFactory::keys();
  for (QString& s : available_styles) s = s.toLower();
  if (!theme.isEmpty() && available_styles.contains(theme)) {
    QApplication::setStyle(QStyleFactory::create(theme));
  }

  // register meta types
  qRegisterMetaType<QSharedPointer<GpgPassphraseContext> >(
      "QSharedPointer<GpgPassphraseContext>");

  // init signal station
  UISignalStation::GetInstance();

  // init common utils
  CommonUtils::GetInstance();

  // application proxy configure
  auto proxy_enable = settings.value("proxy/enable", false).toBool();

  // if enable proxy for application
  if (proxy_enable) {
    try {
      QString proxy_type =
          settings.value("proxy/proxy_type", QString{}).toString();
      QString proxy_host =
          settings.value("proxy/proxy_host", QString{}).toString();
      int proxy_port = settings.value("prox/port", 0).toInt();
      QString const proxy_username =
          settings.value("proxy/username", QString{}).toString();
      QString const proxy_password =
          settings.value("proxy/password", QString{}).toString();

      QNetworkProxy::ProxyType proxy_type_qt = QNetworkProxy::NoProxy;
      if (proxy_type == "HTTP") {
        proxy_type_qt = QNetworkProxy::HttpProxy;
      } else if (proxy_type == "Socks5") {
        proxy_type_qt = QNetworkProxy::Socks5Proxy;
      } else {
        proxy_type_qt = QNetworkProxy::DefaultProxy;
      }

      // create proxy object and apply settings
      QNetworkProxy proxy;
      if (proxy_type_qt != QNetworkProxy::DefaultProxy) {
        proxy.setType(proxy_type_qt);
        proxy.setHostName(proxy_host);
        proxy.setPort(proxy_port);
        if (!proxy_username.isEmpty()) {
          proxy.setUser(proxy_username);
        }
        if (!proxy_password.isEmpty()) {
          proxy.setPassword(proxy_password);
        }
      } else {
        proxy.setType(proxy_type_qt);
      }
      QNetworkProxy::setApplicationProxy(proxy);

    } catch (...) {
      FLOG_W("setting operation error: proxy setings");
      // no proxy by default
      QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
    }
  } else {
    // no proxy by default
    QNetworkProxy::setApplicationProxy(QNetworkProxy::NoProxy);
  }
}

void WaitingAllInitializationFinished() {
  if (Module::RetrieveRTValueTypedOrDefault<>("core", "env.state.all", 0) ==
      0) {
    LOG_D() << "ui init is done, but core doesn't, going to wait for core...";
    WaitEnvCheckingProcess();
  }
  LOG_D() << "application fully initialized...";
}

auto RunGpgFrontendUI(QApplication* app) -> int {
  // create main window and show it
  auto main_window = QSharedPointer<GpgFrontend::UI::MainWindow>::create();
  main_window->setAttribute(Qt::WA_DeleteOnClose, false);

  // pre-check, if application need to restart
  if (CommonUtils::GetInstance()->IsApplicationNeedRestart()) {
    FLOG_D("application need to restart, before main window init.");
    return kDeepRestartCode;
  }

  LOG_D() << "main window start to initialize...";

  // init main window
  main_window->Init();

  // show main windows
  main_window->show();

  return QApplication::exec();
}

void GPGFRONTEND_UI_EXPORT DestroyGpgFrontendUI() {}

/**
 * @brief setup the locale and load the translations
 *
 */
void InitUITranslations() {
  for (const auto& translator : registered_translators) {
    QCoreApplication::removeTranslator(translator);
  }
  registered_translators.clear();
  loaded_qm_datum.clear();

  auto* translator = new QTranslator(QCoreApplication::instance());
  if (translator->load(QLocale(), QLatin1String("qt"), QLatin1String("_"),
                       QLatin1String(":/i18n_qt"), QLatin1String(".qm"))) {
    QCoreApplication::installTranslator(translator);
    registered_translators.append(translator);
  }

  translator = new QTranslator(QCoreApplication::instance());
  if (translator->load(QLocale(), QLatin1String("qtbase"), QLatin1String("_"),
                       QLatin1String(":/i18n_qt"), QLatin1String(".qm"))) {
    QCoreApplication::installTranslator(translator);
    registered_translators.append(translator);
  }

  translator = new QTranslator(QCoreApplication::instance());
  if (translator->load(QLocale(), QLatin1String(PROJECT_NAME),
                       QLatin1String("."), QLatin1String(":/i18n"),
                       QLatin1String(".qm"))) {
    QCoreApplication::installTranslator(translator);
    registered_translators.append(translator);
  }
}

void InitModulesTranslations() {
  // register module's translations
  UIModuleManager::GetInstance().RegisterAllModuleTranslators();
  // translate all params
  UIModuleManager::GetInstance().TranslateAllModulesParams();
}

}  // namespace GpgFrontend::UI
