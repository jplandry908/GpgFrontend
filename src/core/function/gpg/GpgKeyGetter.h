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

#include "core/function/basic/GpgFunctionObject.h"
#include "core/model/GpgKeyTableModel.h"
#include "core/typedef/GpgTypedef.h"

namespace GpgFrontend {

/**
 * @brief
 *
 */
class GPGFRONTEND_CORE_EXPORT GpgKeyGetter
    : public SingletonFunctionObject<GpgKeyGetter> {
 public:
  /**
   * @brief Construct a new Gpg Key Getter object
   *
   * @param channel
   */
  explicit GpgKeyGetter(int channel = kGpgFrontendDefaultChannel);

  /**
   * @brief Destroy the Gpg Key Getter object
   *
   */
  ~GpgKeyGetter();

  /**
   * @brief Get the Key object
   *
   * @param fpr
   * @return GpgKey
   */
  auto GetKey(const QString& key_id, bool use_cache = true) -> GpgKey;

  /**
   * @brief Get the Key Ptr object
   *
   * @param key_id
   * @param use_cache
   * @return QSharedPointer<GpgKey>
   */
  auto GetKeyPtr(const QString& key_id,
                 bool use_cache = true) -> QSharedPointer<GpgKey>;

  /**
   * @brief Get the Pubkey object
   *
   * @param fpr
   * @return GpgKey
   */
  auto GetPubkey(const QString& key_id, bool use_cache = true) -> GpgKey;

  /**
   * @brief Get the Pubkey Ptr object
   *
   * @param key_id
   * @param use_cache
   * @return GpgKeyPtr
   */
  auto GetPubkeyPtr(const QString& key_id, bool use_cache = true) -> GpgKeyPtr;

  /**
   * @brief Get the Pubkey Ptr object
   *
   * @param key_id
   * @param use_cache
   * @return GpgKeyPtr
   */
  auto GetKeyORSubkeyPtr(const QString& key_id) -> GpgAbstractKeyPtr;

  /**
   * @brief
   *
   * @return QContainer<QSharedPointer<GpgKey>>
   */
  auto Fetch() -> QContainer<QSharedPointer<GpgKey>>;

  /**
   * @brief flush the keys in the cache
   *
   */
  auto FlushKeyCache() -> bool;

  /**
   * @brief Get the Keys object
   *
   * @param ids
   * @return KeyListPtr
   */
  auto GetKeys(const KeyIdArgsList& key_ids) -> GpgKeyList;

 private:
  class Impl;
  SecureUniquePtr<Impl> p_;
};
}  // namespace GpgFrontend
