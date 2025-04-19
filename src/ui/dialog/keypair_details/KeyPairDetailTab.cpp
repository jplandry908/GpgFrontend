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

#include "KeyPairDetailTab.h"

#include "core/function/GlobalSettingStation.h"
#include "core/function/gpg/GpgKeyGetter.h"
#include "core/model/GpgKey.h"
#include "core/module/ModuleManager.h"
#include "core/utils/CommonUtils.h"
#include "ui/UISignalStation.h"

namespace GpgFrontend::UI {
KeyPairDetailTab::KeyPairDetailTab(int channel, GpgKeyPtr key, QWidget* parent)
    : QWidget(parent),
      current_gpg_context_channel_(channel),
      key_(std::move(key)) {
  assert(key_->IsGood());

  owner_box_ = new QGroupBox(tr("Owner"));
  key_box_ = new QGroupBox(tr("Primary Key"));
  fingerprint_box_ = new QGroupBox(tr("Fingerprint"));
  additional_uid_box_ = new QGroupBox(tr("Additional UIDs"));

  name_var_label_ = new QLabel();
  name_var_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  email_var_label_ = new QLabel();
  email_var_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

  comment_var_label_ = new QLabel();
  comment_var_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  key_id_var_label_ = new QLabel();
  key_id_var_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);

  usage_var_label_ = new QLabel();

  owner_trust_var_label_ = new QLabel();
  key_size_var_label_ = new QLabel();
  expire_var_label_ = new QLabel();
  created_var_label_ = new QLabel();
  last_update_var_label_ = new QLabel();
  algorithm_var_label_ = new QLabel();
  algorithm_detail_var_label_ = new QLabel();
  primary_key_exist_var_label_ = new QLabel();

  auto* mvbox = new QVBoxLayout();
  auto* vbox_kd = new QGridLayout();
  auto* vbox_od = new QGridLayout();

  vbox_od->addWidget(new QLabel(tr("Name") + ": "), 0, 0);
  vbox_od->addWidget(new QLabel(tr("Email Address") + ": "), 1, 0);
  vbox_od->addWidget(new QLabel(tr("Comment") + ": "), 2, 0);
  vbox_od->addWidget(name_var_label_, 0, 1);
  vbox_od->addWidget(email_var_label_, 1, 1);
  vbox_od->addWidget(comment_var_label_, 2, 1);

  vbox_kd->addWidget(new QLabel(tr("Key ID") + ": "), 0, 0);
  vbox_kd->addWidget(new QLabel(tr("Algorithm") + ": "), 1, 0);
  vbox_kd->addWidget(new QLabel(tr("Algorithm Detail") + ": "), 2, 0);
  vbox_kd->addWidget(new QLabel(tr("Key Size") + ": "), 3, 0);
  vbox_kd->addWidget(new QLabel(tr("Usage") + ": "), 4, 0);
  vbox_kd->addWidget(new QLabel(tr("Owner Trust Level") + ": "), 5, 0);
  vbox_kd->addWidget(new QLabel(tr("Create Date (Local Time)") + ": "), 6, 0);
  vbox_kd->addWidget(new QLabel(tr("Expires on (Local Time)") + ": "), 7, 0);
  vbox_kd->addWidget(new QLabel(tr("Last Update (Local Time)") + ": "), 8, 0);
  vbox_kd->addWidget(new QLabel(tr("Primary Key Existence") + ": "), 9, 0);

  key_id_var_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  vbox_kd->addWidget(key_id_var_label_, 0, 1, 1, 1);
  vbox_kd->addWidget(algorithm_var_label_, 1, 1, 1, 2);
  vbox_kd->addWidget(algorithm_detail_var_label_, 2, 1, 1, 2);
  vbox_kd->addWidget(key_size_var_label_, 3, 1, 1, 2);
  vbox_kd->addWidget(usage_var_label_, 4, 1, 1, 2);
  vbox_kd->addWidget(owner_trust_var_label_, 5, 1, 1, 2);
  vbox_kd->addWidget(created_var_label_, 6, 1, 1, 2);
  vbox_kd->addWidget(expire_var_label_, 7, 1, 1, 2);
  vbox_kd->addWidget(last_update_var_label_, 8, 1, 1, 2);
  vbox_kd->addWidget(primary_key_exist_var_label_, 9, 1, 1, 2);

  auto* copy_key_id_button = new QPushButton(tr("Copy"));
  copy_key_id_button->setFlat(true);
  copy_key_id_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
  vbox_kd->addWidget(copy_key_id_button, 0, 2);
  connect(copy_key_id_button, &QPushButton::clicked, this, [=]() {
    QString fpr = key_id_var_label_->text().trimmed();
    QClipboard* cb = QApplication::clipboard();
    cb->setText(fpr);
  });

  owner_box_->setLayout(vbox_od);
  mvbox->addWidget(owner_box_);
  key_box_->setLayout(vbox_kd);
  mvbox->addWidget(key_box_);

  fingerprint_var_label_ = new QLabel();
  fingerprint_var_label_->setWordWrap(false);
  fingerprint_var_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  fingerprint_var_label_->setStyleSheet("margin-left: 0; margin-right: 5;");
  fingerprint_var_label_->setAlignment(Qt::AlignCenter);
  fingerprint_var_label_->setMinimumWidth(400);
  auto* hbox_fp = new QHBoxLayout();

  hbox_fp->addStretch();
  hbox_fp->addWidget(fingerprint_var_label_);

  auto* copy_fingerprint_button = new QPushButton(tr("Copy"));
  copy_fingerprint_button->setFlat(true);
  copy_fingerprint_button->setToolTip(tr("copy fingerprint to clipboard"));
  connect(copy_fingerprint_button, &QPushButton::clicked, this,
          &KeyPairDetailTab::slot_copy_fingerprint);

  hbox_fp->addWidget(copy_fingerprint_button);
  hbox_fp->addStretch();

  fingerprint_box_->setLayout(hbox_fp);
  mvbox->addWidget(fingerprint_box_);
  mvbox->addStretch();

  auto* exp_box = new QHBoxLayout();
  QPixmap pixmap(":/icons/warning.png");

  exp_label_ = new QLabel();
  icon_label_ = new QLabel();

  icon_label_->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio));
  exp_label_->setAlignment(Qt::AlignCenter);
  exp_box->addStretch();
  exp_box->addWidget(icon_label_);
  exp_box->addWidget(exp_label_);
  exp_box->addStretch();
  mvbox->addLayout(exp_box);
  mvbox->setContentsMargins(0, 0, 0, 0);

  // when key database updated
  connect(UISignalStation::GetInstance(),
          &UISignalStation::SignalKeyDatabaseRefreshDone, this,
          &KeyPairDetailTab::slot_refresh_key);

  slot_refresh_key_info();
  setAttribute(Qt::WA_DeleteOnClose, true);
  setLayout(mvbox);
}

void KeyPairDetailTab::slot_copy_fingerprint() {
  QString fpr =
      fingerprint_var_label_->text().trimmed().replace(" ", QString());
  QClipboard* cb = QApplication::clipboard();
  cb->setText(fpr);
}

void KeyPairDetailTab::slot_refresh_key_info() {
  // Show the situation that primary key not exists.
  primary_key_exist_var_label_->setText(
      key_->IsHasMasterKey() ? tr("Exists") : tr("Not Exists"));
  if (!key_->IsHasMasterKey()) {
    auto palette_expired = primary_key_exist_var_label_->palette();
    palette_expired.setColor(primary_key_exist_var_label_->foregroundRole(),
                             Qt::red);
    primary_key_exist_var_label_->setPalette(palette_expired);
  } else {
    auto palette_valid = primary_key_exist_var_label_->palette();
    palette_valid.setColor(primary_key_exist_var_label_->foregroundRole(),
                           Qt::darkGreen);
    primary_key_exist_var_label_->setPalette(palette_valid);
  }

  if (key_->IsExpired()) {
    auto palette_expired = expire_var_label_->palette();
    palette_expired.setColor(expire_var_label_->foregroundRole(), Qt::red);
    expire_var_label_->setPalette(palette_expired);
  } else {
    auto palette_valid = expire_var_label_->palette();
    palette_valid.setColor(expire_var_label_->foregroundRole(), Qt::darkGreen);
    expire_var_label_->setPalette(palette_valid);
  }

  name_var_label_->setText(key_->Name());
  email_var_label_->setText(key_->Email());

  comment_var_label_->setText(key_->Comment());
  key_id_var_label_->setText(key_->ID());

  QString buffer;
  QTextStream usage_steam(&buffer);

  if (key_->IsHasCertCap()) {
    usage_steam << tr("Certificate") << " ";
  }
  if (key_->IsHasEncrCap()) usage_steam << tr("Encrypt") << " ";
  if (key_->IsHasSignCap()) usage_steam << tr("Sign") << " ";
  if (key_->IsHasAuthCap()) usage_steam << tr("Auth") << " ";

  usage_var_label_->setText(usage_steam.readAll());
  owner_trust_var_label_->setText(key_->OwnerTrust());

  QString key_size_val;
  QString key_expire_val;
  QString key_create_time_val;
  QString key_algo_val;
  QString key_algo_detail_val;
  QString key_last_update_val;

  key_size_val = QString::number(key_->PrimaryKeyLength());

  if (key_->ExpirationTime().toSecsSinceEpoch() == 0) {
    expire_var_label_->setText(tr("Never Expire"));
  } else {
    expire_var_label_->setText(QLocale().toString((key_->ExpirationTime())));
  }

  key_algo_val = key_->PublicKeyAlgo();
  key_algo_detail_val = key_->Algo();

  created_var_label_->setText(QLocale().toString(key_->CreationTime()));

  if (key_->LastUpdateTime().toSecsSinceEpoch() == 0) {
    last_update_var_label_->setText(tr("No Data"));
  } else {
    last_update_var_label_->setText(QLocale().toString(key_->LastUpdateTime()));
  }

  key_size_var_label_->setText(key_size_val);
  algorithm_var_label_->setText(key_algo_val);
  algorithm_detail_var_label_->setText(key_algo_detail_val);
  fingerprint_var_label_->setText(BeautifyFingerprint(key_->Fingerprint()));
  fingerprint_var_label_->setWordWrap(true);  // for x448 and ed448

  icon_label_->hide();
  exp_label_->hide();

  if (key_->IsExpired()) {
    slot_refresh_notice(":/icons/warning.png",
                        tr("Warning: The primary key has expired."));
  } else if (key_->IsRevoked()) {
    slot_refresh_notice(":/icons/warning.png",
                        tr("Warning: The primary key has been revoked."));
  } else if (key_->IsPrivateKey() && !key_->IsHasMasterKey()) {
    slot_refresh_notice(":/icons/warning.png",
                        tr("Warning: The primary key is not exists."));
  } else {
    slot_query_key_publish_state();
  }
}

void KeyPairDetailTab::slot_refresh_key() {
  // refresh the key
  auto refreshed_key = GpgKeyGetter::GetInstance(current_gpg_context_channel_)
                           .GetKeyPtr(key_->ID());
  assert(refreshed_key != nullptr);

  std::swap(this->key_, refreshed_key);
  this->slot_refresh_key_info();
}

void KeyPairDetailTab::slot_query_key_publish_state() {
  bool forbid_all_gnupg_connection =
      GetSettings().value("network/forbid_all_gnupg_connection").toBool();

  bool auto_fetch_key_publish_status =
      GetSettings().value("network/auto_fetch_key_publish_status").toBool();

  if (forbid_all_gnupg_connection || !auto_fetch_key_publish_status) return;

  if (!Module::IsModuleActivate(kKeyServerSyncModuleID)) {
    return;
  }

  const auto fpr = key_->Fingerprint();

  Module::TriggerEvent(
      "REQUEST_GET_PUBLIC_KEY_BY_FINGERPRINT",
      {
          {"fingerprint", QString(fpr)},
      },
      [=](Module::EventIdentifier i, Module::Event::ListenerIdentifier ei,
          Module::Event::Params p) {
        LOG_D() << "REQUEST_GET_PUBLIC_KEY_BY_FINGERPRINT callback: " << i
                << ei;

        if (p["ret"] != "0" || !p["error_msg"].isEmpty()) {
          LOG_E() << "An error occurred trying to get data from key:" << fpr
                  << "error message: " << p["error_msg"]
                  << "reply data: " << p["reply_data"];
        } else if (p.contains("key_data")) {
          const auto key_data = p["key_data"];
          LOG_D() << "got key data of key " << fpr
                  << " from key server: " << key_data;

          if (!key_data.isEmpty()) {
            slot_refresh_notice(
                ":/icons/publish.png",
                tr("Notice: The public key has been published on "
                   "keys.openpgp.org."));
          }
        }
      });
}

void KeyPairDetailTab::slot_refresh_notice(const QString& icon,
                                           const QString& info) {
  icon_label_->hide();
  exp_label_->hide();

  if (!icon.isEmpty()) {
    QPixmap pixmap(icon);
    icon_label_->setPixmap(pixmap.scaled(24, 24, Qt::KeepAspectRatio));
    icon_label_->show();
  }

  if (!info.isEmpty()) {
    exp_label_->setText(info);
    exp_label_->show();
  }
}
}  // namespace GpgFrontend::UI
