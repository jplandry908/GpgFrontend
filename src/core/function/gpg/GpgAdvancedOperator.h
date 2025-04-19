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

//
// Created by eric on 07.01.2023.
//

#pragma once

#include "core/function/basic/GpgFunctionObject.h"
#include "core/function/gpg/GpgCommandExecutor.h"
#include "core/function/gpg/GpgComponentManager.h"
#include "core/model/DataObject.h"

namespace GpgFrontend {

class GPGFRONTEND_CORE_EXPORT GpgAdvancedOperator
    : public SingletonFunctionObject<GpgAdvancedOperator> {
 public:
  /**
   * @brief Construct a new Gpg Advanced Operator object
   *
   * @param channel
   */
  explicit GpgAdvancedOperator(int channel);

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  auto ClearGpgPasswordCache() -> bool;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  auto ReloadAllGpgComponents() -> bool;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  auto RestartGpgComponents() -> bool;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  auto ResetConfigures() -> bool;

  /**
   * @brief
   *
   * @return true
   * @return false
   */
  auto LaunchAllGpgComponents() -> bool;

  /**
   * @brief
   *
   */
  auto KillAllGpgComponents() -> bool;

 private:
  GpgCommandExecutor& exec_ =
      GpgCommandExecutor::GetInstance(SingletonFunctionObject::GetChannel());
  GpgContext& ctx_ =
      GpgContext::GetInstance(SingletonFunctionObject::GetChannel());
  GpgComponentManager& mgr_ =
      GpgComponentManager::GetInstance(SingletonFunctionObject::GetChannel());
};

}  // namespace GpgFrontend
