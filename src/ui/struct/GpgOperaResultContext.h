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

#include "core/typedef/CoreTypedef.h"
#include "core/typedef/GpgTypedef.h"
#include "ui/struct/GpgOperaResult.h"

namespace GpgFrontend::UI {

using OperaWaitingHd = std::function<void()>;
using OperaWaitingCb = std::function<void(OperaWaitingHd)>;

struct GpgOperaCategory {
  QStringList paths;
  QStringList o_paths;
  QContainer<GFBuffer> buffers;
};

struct GpgOperaContext;

struct GpgOperaContextBasement {
  QContainer<OperaWaitingCb> operas;
  QContainer<GpgOperaResult> opera_results;
  GpgAbstractKeyPtrList keys;
  GpgAbstractKeyPtrList singer_keys;
  QStringList unknown_fprs;
  bool ascii;

  QMap<int, GpgOperaCategory> categories;

  auto GetContextPath(int category) -> QStringList&;

  auto GetContextOutPath(int category) -> QStringList&;

  auto GetContextBuffer(int category) -> QContainer<GFBuffer>&;

  auto GetAllPath() -> QStringList;

  auto GetAllOutPath() -> QStringList;
};

struct GpgOperaContext {
  QSharedPointer<GpgOperaContextBasement> base;

  QStringList paths;
  QStringList o_paths;
  QContainer<GFBuffer> buffers;

  explicit GpgOperaContext(QSharedPointer<GpgOperaContextBasement> base);
};

auto GetGpgOperaContextFromBasement(
    const QSharedPointer<GpgOperaContextBasement>& base, int category)
    -> QSharedPointer<GpgOperaContext>;

}  // namespace GpgFrontend::UI