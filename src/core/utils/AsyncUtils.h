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

#include "core/GpgFrontendCore.h"
#include "core/thread/Task.h"
#include "core/typedef/CoreTypedef.h"
#include "core/typedef/GpgTypedef.h"

namespace GpgFrontend {

/**
 * @brief
 *
 * @param runnable
 * @param callback
 * @param operation
 * @param minimal_version
 */
auto GF_CORE_EXPORT RunGpgOperaAsync(int channel,
                                     const GpgOperaRunnable& runnable,
                                     const GpgOperationCallback& callback,
                                     const QString& operation,
                                     const QString& minimal_version)
    -> Thread::Task::TaskHandler;

/**
 * @brief
 *
 * @param runnable
 * @param operation
 * @param minimal_version
 * @return std::tuple<GpgError, DataObjectPtr>
 */
auto GF_CORE_EXPORT RunGpgOperaSync(int channel,
                                    const GpgOperaRunnable& runnable,
                                    const QString& operation,
                                    const QString& minimal_version)
    -> std::tuple<GpgError, DataObjectPtr>;

/**
 * @brief
 *
 * @param runnable
 * @param callback
 * @param operation
 */
auto GF_CORE_EXPORT RunIOOperaAsync(const OperaRunnable& runnable,
                                    const OperationCallback& callback,
                                    const QString& operation)
    -> Thread::Task::TaskHandler;

/**
 * @brief
 *
 * @param runnable
 * @param callback
 * @param operation
 * @return Thread::Task::TaskHandler
 */
auto GF_CORE_EXPORT RunOperaAsync(const OperaRunnable& runnable,
                                  const OperationCallback& callback,
                                  const QString& operation)
    -> Thread::Task::TaskHandler;
}  // namespace GpgFrontend