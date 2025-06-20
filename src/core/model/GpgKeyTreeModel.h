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

#pragma once

/**
 * @brief
 *
 */

#include "core/model/GpgAbstractKey.h"
#include "core/model/GpgKey.h"
#include "core/typedef/GpgTypedef.h"

namespace GpgFrontend {

enum class GpgKeyTreeColumn : unsigned int {
  kNONE = 0,
  kTYPE = 1 << 1,
  kIDENTITY = 1 << 2,
  kKEY_ID = 1 << 3,
  kUSAGE = 1 << 4,
  kALGO = 1 << 5,
  kCREATE_DATE = 1 << 6,
  kALL = ~0U
};

inline auto operator|(GpgKeyTreeColumn lhs, GpgKeyTreeColumn rhs)
    -> GpgKeyTreeColumn {
  using T = std::underlying_type_t<GpgKeyTreeColumn>;
  return static_cast<GpgKeyTreeColumn>(static_cast<T>(lhs) |
                                       static_cast<T>(rhs));
}

inline auto operator|=(GpgKeyTreeColumn &lhs, GpgKeyTreeColumn rhs)
    -> GpgKeyTreeColumn & {
  lhs = lhs | rhs;
  return lhs;
}

inline auto operator&(GpgKeyTreeColumn lhs, GpgKeyTreeColumn rhs)
    -> GpgKeyTreeColumn {
  using T = std::underlying_type_t<GpgKeyTreeColumn>;
  return static_cast<GpgKeyTreeColumn>(static_cast<T>(lhs) &
                                       static_cast<T>(rhs));
}

inline auto operator&=(GpgKeyTreeColumn &lhs, GpgKeyTreeColumn rhs)
    -> GpgKeyTreeColumn & {
  lhs = lhs & rhs;
  return lhs;
}

inline auto operator~(GpgKeyTreeColumn hs) -> GpgKeyTreeColumn {
  using T = std::underlying_type_t<GpgKeyTreeColumn>;
  return static_cast<GpgKeyTreeColumn>(~static_cast<T>(hs));
}

enum class GpgKeyTreeDisplayMode : unsigned int {
  kNONE = 0,
  kPUBLIC_KEY = 1 << 0,
  kPRIVATE_KEY = 1 << 1,
  kFAVORITES = 1 << 2,
  kALL = ~0U
};

inline auto operator|(GpgKeyTreeDisplayMode lhs, GpgKeyTreeDisplayMode rhs)
    -> GpgKeyTreeDisplayMode {
  using T = std::underlying_type_t<GpgKeyTreeDisplayMode>;
  return static_cast<GpgKeyTreeDisplayMode>(static_cast<T>(lhs) |
                                            static_cast<T>(rhs));
}

inline auto operator|=(GpgKeyTreeDisplayMode &lhs, GpgKeyTreeDisplayMode rhs)
    -> GpgKeyTreeDisplayMode & {
  lhs = lhs | rhs;
  return lhs;
}

inline auto operator&(GpgKeyTreeDisplayMode lhs, GpgKeyTreeDisplayMode rhs)
    -> bool {
  using T = std::underlying_type_t<GpgKeyTreeDisplayMode>;
  return (static_cast<T>(lhs) & static_cast<T>(rhs)) != 0;
}

class GF_CORE_EXPORT GpgKeyTreeItem {
 public:
  GpgKeyTreeItem() = default;

  /**
   * @brief Construct a new Gpg Key Tree Item object
   *
   * @param headers
   */
  explicit GpgKeyTreeItem(QSharedPointer<GpgAbstractKey> key,
                          QVariantList data);

  /**
   * @brief
   *
   * @param child
   */
  void AppendChild(const QSharedPointer<GpgKeyTreeItem> &child);

  /**
   * @brief
   *
   * @param row
   * @return GpgKeyTreeItem*
   */
  auto Child(int row) -> GpgKeyTreeItem *;

  /**
   * @brief
   *
   * @return qsizetype
   */
  [[nodiscard]] auto ChildCount() const -> qsizetype;

  /**
   * @brief
   *
   * @return qsizetype
   */
  [[nodiscard]] auto ColumnCount() const -> qsizetype;

  /**
   * @brief
   *
   * @param column
   * @return QVariant
   */
  [[nodiscard]] auto Data(qsizetype column) const -> QVariant;

  /**
   * @brief
   *
   * @return qsizetype
   */
  [[nodiscard]] auto Row() const -> qsizetype;

  /**
   * @brief
   *
   * @return GpgKeyTreeItem*
   */
  auto ParentItem() -> GpgKeyTreeItem *;

  /**
   * @brief
   *
   * @return bool
   */
  [[nodiscard]] auto Checked() const -> bool;

  /**
   * @brief Set the Checked object
   *
   * @return true
   * @return false
   */
  void SetChecked(bool);

  /**
   * @brief
   *
   * @return bool
   */
  [[nodiscard]] auto Checkable() const -> bool;

  /**
   * @brief
   *
   * @return bool
   */
  void SetCheckable(bool);

  /**
   * @brief
   *
   * @return bool
   */
  [[nodiscard]] auto Enable() const -> bool;

  /**
   * @brief
   *
   * @return bool
   */
  void SetEnable(bool);

  /**
   * @brief
   *
   * @return bool
   */
  [[nodiscard]] auto Key() const -> GpgAbstractKey *;

  /**
   * @brief
   *
   * @return GpgAbstractKeyPtr
   */
  [[nodiscard]] auto SharedKey() const -> GpgAbstractKeyPtr;

 private:
  QContainer<QSharedPointer<GpgKeyTreeItem>> children_;
  QVariantList data_;
  bool checked_ = false;
  bool checkable_;
  bool enable_;
  QSharedPointer<GpgAbstractKey> key_;
  GpgKeyTreeItem *parent_ = nullptr;
};

class GF_CORE_EXPORT GpgKeyTreeModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  using Detector = std::function<bool(GpgAbstractKey *)>;

  /**
   * @brief Construct a new Gpg Key Table Model object
   *
   * @param keys
   * @param parent
   */
  explicit GpgKeyTreeModel(int channel, const GpgAbstractKeyPtrList &keys,
                           Detector checkable, QObject *parent = nullptr);

  /**
   * @brief
   *
   * @param row
   * @param column
   * @param parent
   * @return QModelIndex
   */
  [[nodiscard]] auto index(int row, int column, const QModelIndex &parent) const
      -> QModelIndex override;

  /**
   * @brief
   *
   * @param parent
   * @return int
   */
  [[nodiscard]] auto rowCount(const QModelIndex &parent) const -> int override;

  /**
   * @brief
   *
   * @param parent
   * @return int
   */
  [[nodiscard]] auto columnCount(const QModelIndex &parent) const
      -> int override;

  /**
   * @brief
   *
   * @param index
   * @param role
   * @return QVariant
   */
  [[nodiscard]] auto data(const QModelIndex &index, int role) const
      -> QVariant override;

  /**
   * @brief
   *
   * @param section
   * @param orientation
   * @param role
   * @return QVariant
   */
  [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                int role) const -> QVariant override;

  /**
   * @brief Set the Data object
   *
   * @param index
   * @param value
   * @param role
   * @return true
   * @return false
   */
  auto setData(const QModelIndex &index, const QVariant &value, int role)
      -> bool override;

  /**
   * @brief
   *
   * @param index
   * @return Qt::ItemFlags
   */
  [[nodiscard]] auto flags(const QModelIndex &index) const
      -> Qt::ItemFlags override;

  /**
   * @brief
   *
   * @param index
   * @return QModelIndex
   */
  [[nodiscard]] auto parent(const QModelIndex &index) const
      -> QModelIndex override;

  /**
   * @brief
   *
   * @return int
   */
  [[nodiscard]] auto GetGpgContextChannel() const -> int;

  /**
   * @brief Get the All Checked Key Ids object
   *
   * @return KeyIdArgsList
   */
  auto GetAllCheckedKeyIds() -> KeyIdArgsList;

  /**
   * @brief Get the All Checked Sub Key object
   *
   * @return QContainer<GpgSubKey>
   */
  auto GetAllCheckedSubKey() -> QContainer<GpgSubKey>;

  /**
   * @brief Get the All Checked Keys object
   *
   * @return GpgAbstractKeyPtrList
   */
  auto GetAllCheckedKeys() -> GpgAbstractKeyPtrList;

  /**
   * @brief Get the Key By Index object
   *
   * @return GpgAbstractKey*
   */
  auto GetKeyByIndex(QModelIndex) -> GpgAbstractKey *;

 signals:

  /**
   * @brief
   *
   * @param key_id
   * @param checked
   */
  void SignalKeyCheckedChanged(GpgAbstractKey *key, bool checked);

 private:
  int gpg_context_channel_;
  QVariantList column_headers_;
  Detector checkable_detector_;

  QSharedPointer<GpgKeyTreeItem> root_;
  QContainer<QSharedPointer<GpgKeyTreeItem>> cached_items_;

  /**
   * @brief
   *
   */
  void setup_model_data(const GpgAbstractKeyPtrList &keys);

  /**
   * @brief Create a gpg key tree items object
   *
   * @param key
   * @return QSharedPointer<GpgKeyTreeItem>
   */
  auto create_gpg_key_tree_items(const GpgAbstractKeyPtr &key)
      -> QSharedPointer<GpgKeyTreeItem>;
};

}  // namespace GpgFrontend