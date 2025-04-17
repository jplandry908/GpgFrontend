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

#include "SettingsKeyServer.h"

#include <cstddef>

#include "core/function/GlobalSettingStation.h"
#include "core/model/SettingsObject.h"
#include "core/thread/Task.h"
#include "core/thread/TaskRunnerGetter.h"
#include "ui/struct/settings_object/KeyServerSO.h"
#include "ui/thread/ListedKeyServerTestTask.h"
#include "ui_KeyServerSettings.h"

namespace GpgFrontend::UI {

KeyserverTab::KeyserverTab(QWidget* parent)
    : QWidget(parent),
      ui_(GpgFrontend::SecureCreateSharedObject<Ui_KeyServerSettings>()) {
  ui_->setupUi(this);
  ui_->keyServerListTable->setSizeAdjustPolicy(
      QAbstractScrollArea::AdjustToContents);

  connect(ui_->addKeyServerPushButton, &QPushButton::clicked, this,
          &KeyserverTab::slot_add_key_server);
  connect(ui_->testKeyServerButton, &QPushButton::clicked, this,
          &KeyserverTab::slot_test_listed_key_server);

  ui_->keyServerListGroupBox->setTitle(tr("Keyserver List"));
  ui_->operationsGroupBox->setTitle(tr("Operations"));

  ui_->keyServerListTable->horizontalHeaderItem(0)->setText(tr("Default"));
  ui_->keyServerListTable->horizontalHeaderItem(1)->setText(
      tr("Keyserver Address"));
  ui_->keyServerListTable->horizontalHeaderItem(2)->setText(tr("Security"));
  ui_->keyServerListTable->horizontalHeaderItem(3)->setText(tr("Available"));

  ui_->addKeyServerPushButton->setText(tr("Add"));
  ui_->testKeyServerButton->setText(tr("Test Listed Keyserver"));

  ui_->tipsLabel->setText(
      tr("Tips: Please Double-click table item to edit it."));
  ui_->actionDelete_Selected_Key_Server->setText(tr("Delete Selected"));
  ui_->actionDelete_Selected_Key_Server->setToolTip(
      tr("Delete Selected Key Server"));
  ui_->actionSet_As_Default->setText(tr("Set As Default"));
  ui_->actionSet_As_Default->setToolTip(tr("Set As Default"));

  popup_menu_ = new QMenu(this);
  popup_menu_->addAction(ui_->actionSet_As_Default);
  popup_menu_->addAction(ui_->actionDelete_Selected_Key_Server);

  connect(ui_->keyServerListTable, &QTableWidget::itemChanged,
          [=](QTableWidgetItem* item) {
            if (item->column() != 1) return;
            const auto row_size = ui_->keyServerListTable->rowCount();
            // Update Actions
            if (row_size > 0) {
              key_server_str_list_.clear();
              for (int i = 0; i < row_size; i++) {
                const auto key_server =
                    ui_->keyServerListTable->item(i, 1)->text();
                key_server_str_list_.append(key_server);
              }
            }
          });

  connect(ui_->actionSet_As_Default, &QAction::triggered, [=]() {
    const auto row_size = ui_->keyServerListTable->rowCount();
    for (int i = 0; i < row_size; i++) {
      auto* const item = ui_->keyServerListTable->item(i, 1);
      if (!item->isSelected()) continue;
      this->default_key_server_ = item->text();
    }
    this->slot_refresh_table();
  });

  connect(ui_->actionDelete_Selected_Key_Server, &QAction::triggered, [=]() {
    const auto row_size = ui_->keyServerListTable->rowCount();
    for (int i = 0; i < row_size; i++) {
      const auto item = ui_->keyServerListTable->item(i, 1);
      if (!item->isSelected()) continue;
      this->key_server_str_list_.removeAt(i);
      break;
    }
    this->slot_refresh_table();
  });

  // Read key-list from ini-file and fill it into combobox
  SetSettings();
  slot_refresh_table();
}

void KeyserverTab::SetSettings() {
  KeyServerSO key_server(SettingsObject("key_server"));

  if (key_server.server_list.empty()) key_server.ResetDefaultServerList();
  for (const auto& key_server : key_server.server_list) {
    this->key_server_str_list_.append(key_server);
  }

  const auto default_key_server = key_server.GetTargetServer();
  if (!key_server_str_list_.contains(default_key_server)) {
    key_server_str_list_.append(default_key_server);
  }
  default_key_server_ = default_key_server;
}

void KeyserverTab::slot_add_key_server() {
  auto target_url = ui_->addKeyServerEdit->text();
  if (url_reg_.match(target_url).hasMatch()) {
    if (target_url.startsWith("https://")) {
      ;
    } else if (target_url.startsWith("http://")) {
      QMessageBox::warning(
          this, tr("Insecure keyserver address"),
          tr("For security reasons, using HTTP as the communication protocol "
             "with "
             "the key server is not recommended. It is recommended to use "
             "HTTPS."));
    }
    key_server_str_list_.append(ui_->addKeyServerEdit->text());
  } else {
    auto ret = QMessageBox::warning(
        this, tr("Warning"),
        tr("You may not use HTTPS or HTTP as the protocol for communicating "
           "with the key server, which may not be wrong. But please check the "
           "address you entered again to make sure it is correct. Are you "
           "sure "
           "that want to add it into the keyserver list?"),
        QMessageBox::Ok | QMessageBox::Cancel);

    if (ret == QMessageBox::Cancel) return;
    key_server_str_list_.append(ui_->addKeyServerEdit->text());
  }
  slot_refresh_table();
}

void KeyserverTab::ApplySettings() {
  SettingsObject key_server_json("key_server");
  KeyServerSO key_server_so;

  auto& key_server_list = key_server_so.server_list;
  const auto list_size = key_server_str_list_.size();
  for (int i = 0; i < list_size; i++) {
    const auto key_server = key_server_str_list_[i];
    if (default_key_server_ == key_server) {
      LOG_D() << "set default key server as:" << default_key_server_
              << "index: " << i;
      key_server_so.default_server = i;
    }
    key_server_list << key_server;
  }
  LOG_D() << "key server settings json:" << key_server_so.ToJson();
  key_server_json.Store(key_server_so.ToJson());
}

void KeyserverTab::slot_refresh_table() {
  ui_->keyServerListTable->blockSignals(true);
  ui_->keyServerListTable->setRowCount(key_server_str_list_.size());

  int index = 0;
  for (const auto& server : key_server_str_list_) {
    auto* tmp1 =
        new QTableWidgetItem(server == default_key_server_ ? "*" : QString{});
    tmp1->setTextAlignment(Qt::AlignCenter);
    ui_->keyServerListTable->setItem(index, 0, tmp1);
    tmp1->setFlags(tmp1->flags() ^ Qt::ItemIsEditable);

    auto* tmp2 = new QTableWidgetItem(server);
    tmp2->setTextAlignment(Qt::AlignCenter);
    ui_->keyServerListTable->setItem(index, 1, tmp2);

    auto* tmp3 = new QTableWidgetItem(server.startsWith("https") ? tr("true")
                                                                 : tr("false"));
    tmp3->setTextAlignment(Qt::AlignCenter);
    ui_->keyServerListTable->setItem(index, 2, tmp3);
    tmp3->setFlags(tmp3->flags() ^ Qt::ItemIsEditable);

    auto* tmp4 = new QTableWidgetItem(tr("unknown"));
    tmp4->setTextAlignment(Qt::AlignCenter);
    ui_->keyServerListTable->setItem(index, 3, tmp4);
    tmp4->setFlags(tmp3->flags() ^ Qt::ItemIsEditable);
    index++;
  }
  const auto column_count = ui_->keyServerListTable->columnCount();
  for (int i = 0; i < column_count; i++) {
    ui_->keyServerListTable->resizeColumnToContents(i);
  }
  ui_->keyServerListTable->blockSignals(false);
}

void KeyserverTab::slot_test_listed_key_server() {
  auto timeout = QInputDialog::getInt(this, tr("Set TCP Timeout"),
                                      tr("timeout(ms): "), 2500, 200, 16000);

  QStringList urls;
  const auto row_size = ui_->keyServerListTable->rowCount();
  for (int i = 0; i < row_size; i++) {
    const auto keyserver_url = ui_->keyServerListTable->item(i, 1)->text();
    urls.push_back(keyserver_url);
  }

  auto* task = new ListedKeyServerTestTask(urls, timeout, this);

  connect(
      task,
      &GpgFrontend::UI::ListedKeyServerTestTask::SignalKeyServerListTestResult,
      this,
      [=](QContainer<ListedKeyServerTestTask::KeyServerTestResultType> result) {
        const auto row_size = ui_->keyServerListTable->rowCount();
        if (result.size() != static_cast<qsizetype>(row_size)) return;
        ui_->keyServerListTable->blockSignals(true);
        for (auto i = 0; i < row_size; i++) {
          const auto status = result[i];
          auto* status_iem = ui_->keyServerListTable->item(i, 3);
          if (status == ListedKeyServerTestTask::kTEST_RESULT_TYPE_SUCCESS) {
            status_iem->setText(tr("Reachable"));
            status_iem->setForeground(QBrush(QColor::fromRgb(0, 255, 0)));
          } else {
            status_iem->setText(tr("Not Reachable"));
            status_iem->setForeground(QBrush(QColor::fromRgb(255, 0, 0)));
          }
        }
        ui_->keyServerListTable->blockSignals(false);
      });

  // Waiting Dialog
  auto* waiting_dialog = new QProgressDialog(this);
  waiting_dialog->setMaximum(0);
  waiting_dialog->setMinimum(0);
  auto* waiting_dialog_label = new QLabel(
      tr("Test Key Server Connection...") + "<br /><br />" +
      tr("This test only tests the network connectivity of the key "
         "server. Passing the test does not mean that the key server "
         "is functionally available."));
  waiting_dialog_label->setWordWrap(true);
  waiting_dialog->setLabel(waiting_dialog_label);
  waiting_dialog->resize(420, 120);
  waiting_dialog->setModal(true);
  connect(task, &Thread::Task::SignalTaskEnd,
          [=]() { waiting_dialog->close(); });
  // Show Waiting Dialog
  waiting_dialog->show();
  waiting_dialog->setFocus();

  Thread::TaskRunnerGetter::GetInstance()
      .GetTaskRunner(Thread::TaskRunnerGetter::kTaskRunnerType_Network)
      ->PostTask(task);

  QEventLoop loop;
  connect(task, &Thread::Task::SignalTaskEnd, &loop, &QEventLoop::quit);
  connect(waiting_dialog, &QProgressDialog::canceled, &loop, &QEventLoop::quit);
  loop.exec();
}

void KeyserverTab::contextMenuEvent(QContextMenuEvent* event) {
  QWidget::contextMenuEvent(event);
  if (ui_->keyServerListTable->selectedItems().length() > 0) {
    popup_menu_->exec(event->globalPos());
  }
}

}  // namespace GpgFrontend::UI
