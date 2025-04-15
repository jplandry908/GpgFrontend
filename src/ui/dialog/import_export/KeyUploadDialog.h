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

#include "core/model/GFBuffer.h"
#include "core/typedef/GpgTypedef.h"
#include "ui/GpgFrontendUI.h"
#include "ui/dialog/GeneralDialog.h"

namespace GpgFrontend::UI {

/**
 * @brief
 *
 */
class KeyUploadDialog : public GeneralDialog {
  Q_OBJECT
 public:
  /**
   * @brief Construct a new Key Upload Dialog object
   *
   * @param keys_ids
   * @param parent
   */
  explicit KeyUploadDialog(int channel, GpgAbstractKeyPtrList keys,
                           QWidget* parent);

 public slots:

  /**
   * @brief
   *
   */
  void SlotUpload();

 private slots:

  /**
   * @brief
   *
   * @param keys_data
   */
  void slot_upload_key_to_server(const GFBuffer&);

  /**
   * @brief
   *
   */
  void slot_upload_finished();

 private:
  int current_gpg_context_channel_;
  GpgAbstractKeyPtrList keys_;  ///<
  QByteArray m_key_data_;       ///<
};

}  // namespace GpgFrontend::UI
