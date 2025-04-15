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

#include "core/function/result_analyse/GpgVerifyResultAnalyse.h"
#include "core/model/GpgKey.h"
#include "core/thread/Task.h"
#include "core/typedef/GpgTypedef.h"

namespace GpgFrontend {
class GpgResultAnalyse;
class GpgImportInformation;
}  // namespace GpgFrontend

namespace GpgFrontend::UI {

class InfoBoardWidget;
class TextEdit;

/**
 * @brief
 *
 * @param parent
 * @param info_board
 * @param error
 * @param verify_result
 */
void show_verify_details(QWidget* parent, InfoBoardWidget* info_board,
                         GpgError error, const GpgVerifyResult& verify_result);

/**
 * @brief
 *
 * @param parent
 * @param verify_res
 */
void ImportUnknownKeyFromKeyserver(QWidget* parent,
                                   const GpgVerifyResultAnalyse& verify_res);
/**
 * @brief
 *
 * @param parent
 * @param waiting_title
 * @param func
 */
void process_operation(QWidget* parent, const QString& waiting_title,
                       Thread::Task::TaskRunnable func,
                       Thread::Task::TaskCallback callback = nullptr,
                       DataObjectPtr data_object = nullptr);

/**
 * @brief
 *
 */
class CommonUtils : public QWidget {
  Q_OBJECT
 public:
  /**
   * @brief
   *
   */
  using ImportCallbackFunction =
      std::function<void(const QString&, const QString&, size_t, size_t)>;

  /**
   * @brief Construct a new Common Utils object
   *
   */
  CommonUtils();

  /**
   * @brief Get the Instance object
   *
   * @return CommonUtils*
   */
  static auto GetInstance() -> CommonUtils*;

  /**
   * @brief
   *
   * @param err
   */
  static void RaiseMessageBox(QWidget* parent, GpgError err);

  /**
   * @brief
   *
   * @param err
   */
  static void RaiseFailureMessageBox(QWidget* parent, GpgError err);

  /**
   * @brief
   *
   */
  auto IsApplicationNeedRestart() -> bool;

  /**
   * @brief
   *
   */
  auto KeyExistsInFavoriteList(const QString& key_db_name,
                               const GpgKey& key) -> bool;

  /**
   * @brief
   *
   */
  void AddKey2Favorite(const QString& key_db_name,
                       const GpgAbstractKeyPtr& key);

  /**
   * @brief
   *
   */
  void RemoveKeyFromFavorite(const QString& key_db_name,
                             const GpgAbstractKeyPtr& key);

  /**
   * @brief
   *
   */
  void ImportGpgKeyFromKeyServer(int channel, const GpgKeyPtrList&);

  /**
   * @brief
   *
   * @param parent
   * @param key
   */
  static void OpenDetailsDialogByKey(QWidget* parent, int channel,
                                     const GpgAbstractKeyPtr& key);

 signals:
  /**
   * @brief
   *
   */
  void SignalKeyStatusUpdated();

  /**
   * @brief
   *
   */
  void SignalBadGnupgEnv(QString);

  /**
   * @brief emit when the key database is refreshed
   *
   */
  void SignalKeyDatabaseRefreshDone();

  /**
   * @brief
   *
   */
  void SignalRestartApplication(int);

  /**
   * @brief
   *
   */
  void SignalFavoritesChanged();

 public slots:

  /**
   * @brief
   *
   * @param parent
   * @param in_buffer
   */
  void SlotImportKeys(QWidget* parent, int channel,
                      const QByteArray& in_buffer);

  /**
   * @brief
   *
   * @param parent
   */
  void SlotImportKeyFromFile(QWidget* parent, int channel);

  /**
   * @brief
   *
   * @param parent
   */
  void SlotImportKeyFromKeyServer(QWidget* parent, int channel);

  /**
   * @brief
   *
   * @param parent
   */
  void SlotImportKeyFromClipboard(QWidget* parent, int channel);

  /**
   * @brief
   *
   * @param channel
   */
  void ImportKeyByKeyServerSyncModule(QWidget* parent, int channel,
                                      const QStringList& fprs);

  /**
   * @brief
   *
   * @param ctx_channel
   * @param key_ids
   * @param callback
   */
  static void SlotImportKeyFromKeyServer(
      int channel, const GpgFrontend::KeyIdArgsList& key_ids,
      const CommonUtils::ImportCallbackFunction& callback);

  /**
   * @brief
   *
   * @param arguments
   * @param interact_func
   */
  void SlotExecuteGpgCommand(
      const QStringList& arguments,
      const std::function<void(QProcess*)>& interact_func);

  /**
   * @brief
   *
   * @param arguments
   * @param interact_func
   */
  void SlotExecuteCommand(const QString& cmd, const QStringList& arguments,
                          const std::function<void(QProcess*)>& interact_func);

  /**
   * @brief
   *
   */
  void SlotRestartApplication(int);

 private slots:

  /**
   * @brief update the key status when signal is emitted
   *
   */
  void slot_update_key_status();

  /**
   * @brief
   *
   */
  void slot_update_key_from_server_finished(
      int channel, bool, QString, QByteArray,
      std::shared_ptr<GpgImportInformation>);

 private:
  static QScopedPointer<CommonUtils> instance;  ///<

  bool application_need_to_restart_at_once_ = false;
};

}  // namespace GpgFrontend::UI
