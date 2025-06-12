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

#include "core/function/SecureRandomGenerator.h"
#include "core/function/basic/GpgFunctionObject.h"

namespace GpgFrontend {

/**
 * @brief The PassphraseGenerator class
 *
 * This class is used to generate a passphrase.
 */
class GF_CORE_EXPORT PassphraseGenerator
    : public SingletonFunctionObject<PassphraseGenerator> {
 public:
  /**
   * @brief PassphraseGenerator constructor
   *
   * @param channel The channel to use
   */
  explicit PassphraseGenerator(
      int channel = SingletonFunctionObject::GetDefaultChannel());

  /**
   * @brief generate passphrase
   *
   * @param len length of the passphrase
   * @return QString passphrase
   */
  auto Generate(int len) -> GFBufferOrNone;

  /**
   * @brief generate passphrase
   *
   * @param len length of the passphrase
   * @return QString passphrase
   */
  auto GenerateBytes(int len) -> GFBufferOrNone;

  /**
   * @brief generate passphrase
   *
   * @param len length of the passphrase
   * @return QString passphrase
   */
  static auto GenerateBytesByOpenSSL(int len) -> GFBufferOrNone;

 private:
  SecureRandomGenerator& rand_ =
      SecureRandomGenerator::GetInstance(SingletonFunctionObject::GetChannel());
};

}  // namespace GpgFrontend
