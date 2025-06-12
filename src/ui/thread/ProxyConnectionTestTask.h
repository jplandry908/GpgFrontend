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

#include "core/thread/ThreadingModel.h"

class QNetworkAccessManager;
class QNetworkReply;

namespace GpgFrontend::UI {

/**
 * @brief
 *
 */
class ProxyConnectionTestTask : public Thread::Task {
  Q_OBJECT
 public:
  /**
   * @brief Construct a new Proxy Connection Test Thread object
   *
   * @param url
   * @param timeout
   * @param parent
   */
  explicit ProxyConnectionTestTask(QString url, int timeout);

  /**
   * @brief
   *
   */
  auto Run() -> int override;

 signals:
  /**
   * @brief
   *
   * @param result
   */
  void SignalProxyConnectionTestResult(const QString& result);

 private slots:

  /**
   * @brief
   *
   * @param reply
   */
  void slot_process_network_reply(QNetworkReply* reply);

 private:
  QString url_;                             ///<
  QString result_;                          ///<
  int timeout_ = 500;                       ///<
  QNetworkAccessManager* network_manager_;  ///<
};

}  // namespace GpgFrontend::UI
