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

#include "GpgKeyTableModel.h"

#include "core/function/gpg/GpgKeyGetter.h"
#include "core/model/GpgKey.h"
#include "core/utils/GpgUtils.h"

namespace GpgFrontend {

GpgKeyTableModel::GpgKeyTableModel(int channel, GpgKeyList keys,
                                   QObject *parent)
    : QAbstractTableModel(parent),
      buffered_keys_(std::move(keys)),
      column_headers_({tr("Select"), tr("Type"), tr("Name"),
                       tr("Email Address"), tr("Usage"), tr("Trust"),
                       tr("Key ID"), tr("Create Date"), tr("Algorithm"),
                       tr("Subkey(s)"), tr("Comment")}),
      gpg_context_channel_(channel),
      key_check_state_(buffered_keys_.size()) {
  LOG_D() << "init gpg key table module at channel: " << gpg_context_channel_
          << "key list size: " << buffered_keys_.size();
}

auto GpgKeyTableModel::rowCount(const QModelIndex & /*parent*/) const -> int {
  return static_cast<int>(buffered_keys_.size());
}

auto GpgKeyTableModel::columnCount(const QModelIndex & /*parent*/) const
    -> int {
  return 11;
}

auto GpgKeyTableModel::data(const QModelIndex &index,
                            int role) const -> QVariant {
  if (!index.isValid() || buffered_keys_.empty()) return {};

  if (role == Qt::CheckStateRole) {
    if (index.column() == 0) {
      return key_check_state_[index.row()] ? Qt::Checked : Qt::Unchecked;
    }
  }

  if (role == Qt::DisplayRole) {
    const auto &key = buffered_keys_.at(index.row());

    switch (index.column()) {
      case 0: {
        return index.row();
      }
      case 1: {
        QString type_sym;
        type_sym += key.IsPrivateKey() ? "pub/sec" : "pub";
        if (key.IsPrivateKey() && !key.IsHasMasterKey()) type_sym += "#";
        if (key.IsHasCardKey()) type_sym += "^";
        return type_sym;
      }
      case 2: {
        return key.GetName();
      }
      case 3: {
        return key.GetEmail();
      }
      case 4: {
        return GetUsagesByKey(key);
      }
      case 5: {
        return key.GetOwnerTrust();
      }
      case 6: {
        return key.GetId();
      }
      case 7: {
        return QLocale().toString(key.GetCreateTime(), "yyyy-MM-dd");
      }
      case 8: {
        return key.GetKeyAlgo();
      }
      case 9: {
        return static_cast<int>(key.GetSubKeys()->size());
      }
      case 10: {
        return key.GetComment();
      }
      default:
        return {};
    }
  }

  if (role == Qt::TextAlignmentRole) {
    switch (index.column()) {
      case 0:
      case 1:
      case 4:
      case 5:
      case 6:
      case 7:
      case 9:
        return Qt::AlignCenter;
      default:
        return {};
    }
  }

  return {};
}

auto GpgKeyTableModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant {
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      return column_headers_[section];
    }
  }
  return {};
}

auto GpgKeyTableModel::flags(const QModelIndex &index) const -> Qt::ItemFlags {
  if (!index.isValid()) return Qt::NoItemFlags;

  if (index.column() == 0) {
    return Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }

  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

auto GpgKeyTableModel::setData(const QModelIndex &index, const QVariant &value,
                               int role) -> bool {
  if (!index.isValid()) return false;

  if (index.column() == 0 && role == Qt::CheckStateRole) {
    key_check_state_[index.row()] = (value == Qt::Checked);
    emit dataChanged(index, index);
    return true;
  }

  return false;
}

auto GpgKeyTableModel::GetAllKeyIds() -> KeyIdArgsList {
  KeyIdArgsList keys;
  for (auto &key : buffered_keys_) {
    keys.push_back(key.GetId());
  }
  return keys;
}

auto GpgKeyTableModel::GetKeyIDByRow(int row) const -> QString {
  if (buffered_keys_.size() <= row) return {};

  return buffered_keys_[row].GetId();
}

auto GpgKeyTableModel::IsPrivateKeyByRow(int row) const -> bool {
  if (buffered_keys_.size() <= row) return false;
  return buffered_keys_[row].IsPrivateKey();
}

auto GpgKeyTableModel::GetGpgContextChannel() const -> int {
  return gpg_context_channel_;
}
}  // namespace GpgFrontend
