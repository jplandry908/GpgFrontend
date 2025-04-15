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

#include "ui/widgets/KeyTable.h"

#include "ui/UserInterfaceUtils.h"

namespace GpgFrontend::UI {

auto KeyTable::GetCheckedKeys() const -> GpgAbstractKeyPtrList {
  auto ret = GpgAbstractKeyPtrList{};
  for (decltype(GetRowCount()) i = 0; i < GetRowCount(); i++) {
    if (IsRowChecked(i)) {
      ret.push_back(GetKeyByIndex(model()->index(i, 0)));
    }
  }
  return ret;
}

KeyTable::KeyTable(QWidget* parent, QSharedPointer<GpgKeyTableModel> model,
                   GpgKeyTableDisplayMode select_type,
                   GpgKeyTableColumn column_filter,
                   GpgKeyTableProxyModel::KeyFilter filter)
    : QTableView(parent),
      model_(std::move(model)),
      proxy_model_(model_, select_type, column_filter, std::move(filter), this),
      column_filter_(column_filter) {
  setModel(&proxy_model_);

  verticalHeader()->hide();
  horizontalHeader()->setStretchLastSection(false);
  horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
  setShowGrid(false);
  sortByColumn(2, Qt::AscendingOrder);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::SingleSelection);

  // table items not editable
  setEditTriggers(QAbstractItemView::NoEditTriggers);

  setFocusPolicy(Qt::NoFocus);
  setAlternatingRowColors(true);
  setSortingEnabled(true);

  connect(CommonUtils::GetInstance(), &CommonUtils::SignalFavoritesChanged,
          &proxy_model_, &GpgKeyTableProxyModel::SignalFavoritesChanged);
  connect(this, &KeyTable::SignalColumnTypeChange, this,
          [=](GpgKeyTableColumn global_column_filter) {
            emit(&proxy_model_)
                ->SignalColumnTypeChange(column_filter_ & global_column_filter);
          });
  connect(this, &QTableView::doubleClicked, this,
          [this](const QModelIndex& index) {
            if (!index.isValid()) return;

            auto key = GetKeyByIndex(index);
            if (key == nullptr) return;

            CommonUtils::OpenDetailsDialogByKey(
                this, model_->GetGpgContextChannel(), key);
          });

  connect(&proxy_model_, &GpgKeyTableProxyModel::dataChanged, this,
          [=](const QModelIndex&, const QModelIndex&,
              const QContainer<int>& roles) {
            if (!roles.contains(Qt::CheckStateRole)) return;
            emit SignalKeyChecked();
          });
}

void KeyTable::SetFilterKeyword(const QString& keyword) {
  proxy_model_.SetSearchKeywords(keyword);
}

void KeyTable::RefreshModel(QSharedPointer<GpgKeyTableModel> model) {
  model_ = std::move(model);
  proxy_model_.ResetGpgKeyTableModel(model_);
}

auto KeyTable::IsRowChecked(int row) const -> bool {
  auto index = model()->index(row, 0);
  return index.data(Qt::CheckStateRole).toInt() == Qt::Checked;
}

auto KeyTable::GetRowCount() const -> int { return model()->rowCount(); }

auto KeyTable::GetKeyByIndex(QModelIndex index) const -> GpgAbstractKeyPtr {
  auto idx = proxy_model_.mapToSource(index);
  auto* i = idx.isValid() ? static_cast<GpgKeyTableItem*>(idx.internalPointer())
                          : nullptr;
  assert(i != nullptr);

  return i->SharedKey();
}

auto KeyTable::GetSelectedKeys() const -> GpgAbstractKeyPtrList {
  GpgAbstractKeyPtrList ret;
  QItemSelectionModel* select = selectionModel();
  for (auto index : select->selectedRows()) {
    ret.push_back(GetKeyByIndex(index));
  }
  return ret;
}

void KeyTable::SetRowChecked(int row) const {
  if (row < 0 || row >= model()->rowCount()) return;
  model()->setData(model()->index(row, 0), Qt::Checked, Qt::CheckStateRole);
}

void KeyTable::CheckAll() {
  for (int row = 0; row < model()->rowCount(); ++row) {
    auto index = model()->index(row, 0);
    model()->setData(index, Qt::Checked, Qt::CheckStateRole);
  }
}

void KeyTable::UncheckAll() {
  for (int row = 0; row < model()->rowCount(); ++row) {
    auto index = model()->index(row, 0);
    model()->setData(index, Qt::Unchecked, Qt::CheckStateRole);
  }
}

[[nodiscard]] auto KeyTable::GetRowSelected() const -> int {
  auto selected_indexes = selectedIndexes();
  if (selected_indexes.empty()) return -1;

  return selected_indexes.first().row();
}

void KeyTable::SetFilter(const GpgKeyTableProxyModel::KeyFilter& filter) {
  proxy_model_.SetFilter(filter);
}

void KeyTable::RefreshProxyModel() { proxy_model_.invalidate(); }
}  // namespace GpgFrontend::UI