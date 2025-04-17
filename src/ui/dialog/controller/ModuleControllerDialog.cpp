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

#include "ModuleControllerDialog.h"

#include "core/function/GlobalSettingStation.h"
#include "core/model/SettingsObject.h"
#include "core/struct/settings_object/ModuleSO.h"
#include "ui_ModuleControllerDialog.h"

//
#include "core/module/ModuleManager.h"
#include "ui/widgets/ModuleListView.h"

namespace GpgFrontend::UI {

ModuleControllerDialog::ModuleControllerDialog(QWidget* parent)
    : GeneralDialog("ModuleControllerDialog", parent),
      ui_(QSharedPointer<Ui_ModuleControllerDialog>::create()),
      module_manager_(&Module::ModuleManager::GetInstance()) {
  ui_->setupUi(this);
  ui_->actionsGroupBox->hide();

  ui_->moduleInfoLabel->setText(tr("Module Information"));
  ui_->actionsGroupBox->setTitle(tr("Actions"));
  ui_->showModsDirButton->setText(tr("Show Mods Directory"));

  ui_->tabWidget->setTabText(0, tr("Registered Modules"));
  ui_->tabWidget->setTabText(1, tr("Global Register Table"));
  ui_->tabWidget->setTabText(2, tr("Debugger"));

  ui_->tipsLabel->setText(
      tr("Tips: Module name front with \"*\" stands for integrated module."));

  this->setWindowTitle(tr("Module Controller"));

  connect(ui_->moduleListView, &ModuleListView::SignalSelectModule, this,
          &ModuleControllerDialog::slot_load_module_details);

  connect(ui_->activateOrDeactivateButton, &QPushButton::clicked, this, [=]() {
    auto module_id = ui_->moduleListView->GetCurrentModuleID();
    if (module_id.isEmpty()) return;

    if (!module_manager_->IsModuleActivated(module_id)) {
      module_manager_->ActiveModule(module_id);
    } else {
      module_manager_->DeactivateModule(module_id);
    }

    QTimer::singleShot(1000, [=]() { slot_load_module_details(module_id); });
  });

  connect(ui_->autoActivateButton, &QPushButton::clicked, this, [=]() {
    auto module_id = ui_->moduleListView->GetCurrentModuleID();
    SettingsObject so(QString("module.%1.so").arg(module_id));
    ModuleSO module_so(so);

    module_so.auto_activate =
        ui_->autoActivateButton->text() == tr("Enable Auto Activate");
    so.Store(module_so.ToJson());

    QTimer::singleShot(1000, [=]() { slot_load_module_details(module_id); });
  });

  connect(ui_->triggerEventButton, &QPushButton::clicked, this, [=]() {
    auto event_id =
        QInputDialog::getText(this, "Please provide an Event ID", "Event ID");
    Module::TriggerEvent(event_id);
  });

  connect(ui_->pushButton_4, &QPushButton::clicked, this, []() {

  });

  connect(ui_->showModsDirButton, &QPushButton::clicked, this, [=]() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(
        GlobalSettingStation::GetInstance().GetModulesDir()));
  });

#ifdef RELEASE
  ui_->tabWidget->setTabVisible(2, false);
#endif

  // give user ability to give up all modules
  auto disable_loading_all_modules =
      GetSettings().value("basic/disable_loading_all_modules", false).toBool();
  if (disable_loading_all_modules) {
    ui_->tabWidget->setTabEnabled(0, false);
  }
}

void ModuleControllerDialog::slot_load_module_details(
    Module::ModuleIdentifier module_id) {
  auto module = module_manager_->SearchModule(module_id);
  SettingsObject so(QString("module.%1.so").arg(module_id));
  ModuleSO module_so(so);

  if (module_id.isEmpty() || module == nullptr) {
    ui_->actionsGroupBox->hide();
    return;
  }

  ui_->actionsGroupBox->show();

  if (module_so.module_id != module_id ||
      module_so.module_hash != module->GetModuleHash()) {
    module_so.module_id = module_id;
    module_so.module_hash = module->GetModuleHash();
    module_so.auto_activate = false;
    so.Store(module_so.ToJson());
  }

  QString buffer;
  QTextStream info(&buffer);

  info << "# " << tr("BASIC INFO") << Qt::endl << Qt::endl;

  info << " - " << tr("ID") << ": " << module->GetModuleIdentifier()
       << Qt::endl;
  info << " - " << tr("Version") << ": " << module->GetModuleVersion()
       << Qt::endl;
  info << " - " << tr("SDK Version") << ": " << module->GetModuleSDKVersion()
       << Qt::endl;
  info << " - " << tr("Qt ENV Version") << ": "
       << module->GetModuleQtEnvVersion() << Qt::endl;
  info << " - " << tr("Hash") << ": " << module->GetModuleHash() << Qt::endl;
  info << " - " << tr("Path") << ": " << module->GetModulePath() << Qt::endl;

  auto if_activated = module_manager_->IsModuleActivated(module_id);

  info << " - " << tr("Auto Activate") << ": "
       << (module_so.auto_activate ? tr("True") : tr("False")) << Qt::endl;
  info << " - " << tr("Active") << ": "
       << (if_activated ? tr("True") : tr("False")) << Qt::endl;

  info << Qt::endl;

  info << "# " << tr("METADATA") << Qt::endl << Qt::endl;

#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
  for (const auto& metadata : module->GetModuleMetaData().asKeyValueRange()) {
    info << " - " << metadata.first << ": " << metadata.second << "\n";
  }
#else
  auto map = module->GetModuleMetaData();
  for (auto it = map.keyValueBegin(); it != map.keyValueEnd(); ++it) {
    info << " - " << it->first << ": " << it->second << "\n";
  }
#endif

  info << Qt::endl;

  if (if_activated) {
    info << "# " << tr("Listening Event") << Qt::endl << Qt::endl;

    auto listening_event_ids = module_manager_->GetModuleListening(module_id);
    for (const auto& event_id : listening_event_ids) {
      info << " - " << event_id << "\n";
    }
  }

  ui_->moduleInfoTextBrowser->setText(buffer);
  ui_->activateOrDeactivateButton->setText(if_activated ? tr("Deactivate")
                                                        : tr("Activate"));
  ui_->autoActivateButton->setText(module_so.auto_activate
                                       ? tr("Disable Auto Activate")
                                       : tr("Enable Auto Activate"));
}
}  // namespace GpgFrontend::UI
