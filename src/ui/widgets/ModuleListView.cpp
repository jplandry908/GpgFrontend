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

#include "ModuleListView.h"

#include "core/module/ModuleManager.h"

namespace GpgFrontend::UI {

ModuleListView::ModuleListView(QWidget *parent)
    : QListView(parent), model_(new QStandardItemModel(this)) {
  setModel(model_);
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  load_module_information();
}

void ModuleListView::currentChanged(const QModelIndex &current,
                                    const QModelIndex &previous) {
  QListView::currentChanged(current, previous);
  auto *item = model_->itemFromIndex(current);
  if (item != nullptr) {
    emit this->SignalSelectModule(item->data(Qt::UserRole + 1).toString());
  }
}

void ModuleListView::load_module_information() {
  auto &module_manager = Module::ModuleManager::GetInstance();
  auto module_ids = module_manager.ListAllRegisteredModuleID();

  model_->clear();
  for (const auto &module_id : module_ids) {
    auto module = module_manager.SearchModule(module_id);
    auto integrated_module = module_manager.IsIntegratedModule(module_id);
    auto meta_data = module->GetModuleMetaData();

    auto *item = new QStandardItem((integrated_module ? "*" : "?") +
                                   meta_data.value("Name", module_id));

    item->setData(module_id, Qt::UserRole + 1);
    model_->appendRow(item);
  }
}

auto ModuleListView::GetCurrentModuleID() -> Module::ModuleIdentifier {
  auto *item = model_->itemFromIndex(currentIndex());
  return item != nullptr ? item->data(Qt::UserRole + 1).toString() : "";
}
};  // namespace GpgFrontend::UI
