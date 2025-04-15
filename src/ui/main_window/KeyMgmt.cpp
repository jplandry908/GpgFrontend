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

#include "KeyMgmt.h"

#include "core/function/GlobalSettingStation.h"
#include "core/function/KeyPackageOperator.h"
#include "core/function/gpg/GpgKeyImportExporter.h"
#include "core/function/gpg/GpgKeyOpera.h"
#include "core/model/GpgImportInformation.h"
#include "core/utils/GpgUtils.h"
#include "core/utils/IOUtils.h"
#include "function/SetOwnerTrustLevel.h"
#include "ui/UISignalStation.h"
#include "ui/UserInterfaceUtils.h"
#include "ui/dialog/import_export/ExportKeyPackageDialog.h"
#include "ui/dialog/import_export/KeyImportDetailDialog.h"
#include "ui/dialog/key_generate/KeyGenerateDialog.h"
#include "ui/dialog/key_generate/SubkeyGenerateDialog.h"
#include "ui/function/GpgOperaHelper.h"
#include "ui/main_window/MainWindow.h"
#include "ui/widgets/KeyList.h"

namespace GpgFrontend::UI {

KeyMgmt::KeyMgmt(QWidget* parent)
    : GeneralMainWindow("key_management", parent) {
  /* the list of Keys available*/
  key_list_ = new KeyList(kGpgFrontendDefaultChannel, KeyMenuAbility::kALL,
                          GpgKeyTableColumn::kALL, this);

  key_list_->AddListGroupTab(tr("All"), "all",
                             GpgKeyTableDisplayMode::kPUBLIC_KEY |
                                 GpgKeyTableDisplayMode::kPRIVATE_KEY);

  key_list_->AddListGroupTab(
      tr("Only Public Key"), "only_public_key",
      GpgKeyTableDisplayMode::kPUBLIC_KEY,
      [](const GpgAbstractKey* key) -> bool {
        return !key->IsPrivateKey() &&
               !(key->IsRevoked() || key->IsDisabled() || key->IsExpired());
      });

  key_list_->AddListGroupTab(
      tr("Has Private Key"), "has_private_key",
      GpgKeyTableDisplayMode::kPRIVATE_KEY,
      [](const GpgAbstractKey* key) -> bool {
        return key->IsPrivateKey() &&
               !(key->IsRevoked() || key->IsDisabled() || key->IsExpired());
      });

  key_list_->AddListGroupTab(
      tr("No Primary Key"), "no_primary_key",
      GpgKeyTableDisplayMode::kPUBLIC_KEY |
          GpgKeyTableDisplayMode::kPRIVATE_KEY,
      [](const GpgAbstractKey* key) -> bool {
        if (key->KeyType() != GpgAbstractKeyType::kGPG_KEY) return false;
        return !dynamic_cast<const GpgKey*>(key)->IsHasMasterKey() &&
               !(key->IsRevoked() || key->IsDisabled() || key->IsExpired());
      });

  key_list_->AddListGroupTab(
      tr("Revoked"), "revoked",
      GpgKeyTableDisplayMode::kPUBLIC_KEY |
          GpgKeyTableDisplayMode::kPRIVATE_KEY,

      [](const GpgAbstractKey* key) -> bool { return key->IsRevoked(); });

  key_list_->AddListGroupTab(
      tr("Expired"), "expired",
      GpgKeyTableDisplayMode::kPUBLIC_KEY |
          GpgKeyTableDisplayMode::kPRIVATE_KEY,

      [](const GpgAbstractKey* key) -> bool { return key->IsExpired(); });

  key_list_->AddListGroupTab(
      tr("Disabled"), "disabled",
      GpgKeyTableDisplayMode::kPUBLIC_KEY |
          GpgKeyTableDisplayMode::kPRIVATE_KEY,
      [](const GpgAbstractKey* key) -> bool { return key->IsDisabled(); });

  setCentralWidget(key_list_);

  key_list_->SlotRefresh();

  create_actions();
  create_menus();
  create_tool_bars();

  connect(this, &KeyMgmt::SignalStatusBarChanged,
          qobject_cast<MainWindow*>(this->parent()),
          &MainWindow::SlotSetStatusBarText);

  this->statusBar()->show();

  setWindowTitle(tr("KeyPair Management"));
  setMinimumSize(QSize(640, 480));

  key_list_->AddMenuAction(generate_subkey_act_);
  key_list_->AddMenuAction(delete_selected_keys_act_);
  key_list_->AddSeparator();
  key_list_->AddMenuAction(set_owner_trust_of_key_act_);
  key_list_->AddSeparator();
  key_list_->AddMenuAction(show_key_details_act_);

  connect(this, &KeyMgmt::SignalKeyStatusUpdated,
          UISignalStation::GetInstance(),
          &UISignalStation::SignalKeyDatabaseRefresh);
  connect(UISignalStation::GetInstance(),
          &UISignalStation::SignalRefreshStatusBar, this,
          [=](const QString& message, int timeout) {
            statusBar()->showMessage(message, timeout);
          });
}

void KeyMgmt::create_actions() {
  open_key_file_act_ = new QAction(tr("Open"), this);
  open_key_file_act_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
  open_key_file_act_->setToolTip(tr("Open Key File"));
  connect(open_key_file_act_, &QAction::triggered, this, [this]() {
    CommonUtils::GetInstance()->SlotImportKeyFromFile(
        this, key_list_->GetCurrentGpgContextChannel());
  });

  close_act_ = new QAction(tr("Close"), this);
  close_act_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));
  close_act_->setIcon(QIcon(":/icons/exit.png"));
  close_act_->setToolTip(tr("Close"));
  connect(close_act_, &QAction::triggered, this, &KeyMgmt::close);

  generate_key_pair_act_ = new QAction(tr("New Keypair"), this);
  generate_key_pair_act_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_N));
  generate_key_pair_act_->setIcon(QIcon(":/icons/key_generate.png"));
  generate_key_pair_act_->setToolTip(tr("Generate KeyPair"));
  connect(generate_key_pair_act_, &QAction::triggered, this,
          &KeyMgmt::SlotGenerateKeyDialog);

  generate_subkey_act_ = new QAction(tr("New Subkey"), this);
  generate_subkey_act_->setShortcut(
      QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
  generate_subkey_act_->setIcon(QIcon(":/icons/key_generate.png"));
  generate_subkey_act_->setToolTip(tr("Generate Subkey For Selected KeyPair"));
  connect(generate_subkey_act_, &QAction::triggered, this,
          &KeyMgmt::SlotGenerateSubKey);

  import_key_from_file_act_ = new QAction(tr("File"), this);
  import_key_from_file_act_->setIcon(QIcon(":/icons/import_key_from_file.png"));
  import_key_from_file_act_->setToolTip(tr("Import New Key From File"));
  connect(import_key_from_file_act_, &QAction::triggered, this, [this]() {
    CommonUtils::GetInstance()->SlotImportKeyFromFile(
        this, key_list_->GetCurrentGpgContextChannel());
  });

  import_key_from_clipboard_act_ = new QAction(tr("Clipboard"), this);
  import_key_from_clipboard_act_->setIcon(
      QIcon(":/icons/import_key_from_clipboard.png"));
  import_key_from_clipboard_act_->setToolTip(
      tr("Import New Key From Clipboard"));
  connect(import_key_from_clipboard_act_, &QAction::triggered, this, [this]() {
    CommonUtils::GetInstance()->SlotImportKeyFromClipboard(
        this, key_list_->GetCurrentGpgContextChannel());
  });

  bool const forbid_all_gnupg_connection =
      GetSettings()
          .value("network/forbid_all_gnupg_connection", false)
          .toBool();

  import_key_from_key_server_act_ = new QAction(tr("Keyserver"), this);
  import_key_from_key_server_act_->setIcon(
      QIcon(":/icons/import_key_from_server.png"));
  import_key_from_key_server_act_->setToolTip(
      tr("Import New Key From Keyserver"));
  import_key_from_key_server_act_->setDisabled(forbid_all_gnupg_connection);
  connect(import_key_from_key_server_act_, &QAction::triggered, this, [this]() {
    CommonUtils::GetInstance()->SlotImportKeyFromKeyServer(
        this, key_list_->GetCurrentGpgContextChannel());
  });

  import_keys_from_key_package_act_ = new QAction(tr("Key Package"), this);
  import_keys_from_key_package_act_->setIcon(QIcon(":/icons/key_package.png"));
  import_keys_from_key_package_act_->setToolTip(
      tr("Import Key(s) From a Key Package"));
  connect(import_keys_from_key_package_act_, &QAction::triggered, this,
          &KeyMgmt::SlotImportKeyPackage);

  export_key_to_clipboard_act_ = new QAction(tr("Export To Clipboard"), this);
  export_key_to_clipboard_act_->setIcon(
      QIcon(":/icons/export_key_to_clipboard.png"));
  export_key_to_clipboard_act_->setToolTip(
      tr("Export Checked Key(s) To Clipboard"));
  connect(export_key_to_clipboard_act_, &QAction::triggered, this,
          &KeyMgmt::SlotExportKeyToClipboard);

  export_key_to_file_act_ = new QAction(tr("Export As Key Package"), this);
  export_key_to_file_act_->setIcon(QIcon(":/icons/key_package.png"));
  export_key_to_file_act_->setToolTip(
      tr("Export Checked Key(s) To a Key Package"));
  connect(export_key_to_file_act_, &QAction::triggered, this,
          &KeyMgmt::SlotExportKeyToKeyPackage);

  export_key_as_open_ssh_format_ = new QAction(tr("Export As OpenSSH"), this);
  export_key_as_open_ssh_format_->setIcon(QIcon(":/icons/ssh-key.png"));
  export_key_as_open_ssh_format_->setToolTip(
      tr("Export Checked Key As OpenSSH Format to File"));
  connect(export_key_as_open_ssh_format_, &QAction::triggered, this,
          &KeyMgmt::SlotExportAsOpenSSHFormat);

  delete_selected_keys_act_ = new QAction(tr("Delete Selected Key(s)"), this);
  delete_selected_keys_act_->setIcon(QIcon(":/icons/button_delete.png"));
  delete_selected_keys_act_->setToolTip(tr("Delete the Selected keys"));
  connect(delete_selected_keys_act_, &QAction::triggered, this,
          &KeyMgmt::SlotDeleteSelectedKeys);

  delete_checked_keys_act_ = new QAction(tr("Delete Checked Key(s)"), this);
  delete_checked_keys_act_->setIcon(QIcon(":/icons/button_delete.png"));
  delete_checked_keys_act_->setToolTip(tr("Delete the Checked keys"));

  connect(delete_checked_keys_act_, &QAction::triggered, this,
          &KeyMgmt::SlotDeleteCheckedKeys);

  show_key_details_act_ = new QAction(tr("Show Key Details"), this);
  show_key_details_act_->setIcon(QIcon(":/icons/detail.png"));
  show_key_details_act_->setToolTip(tr("Show Details for this Key"));
  connect(show_key_details_act_, &QAction::triggered, this,
          &KeyMgmt::SlotShowKeyDetails);

  set_owner_trust_of_key_act_ = new QAction(tr("Set Owner Trust Level"), this);
  set_owner_trust_of_key_act_->setIcon(QIcon(":/icons/stairs.png"));
  set_owner_trust_of_key_act_->setToolTip(tr("Set Owner Trust Level"));
  set_owner_trust_of_key_act_->setData(QVariant("set_owner_trust_level"));
  connect(set_owner_trust_of_key_act_, &QAction::triggered, this, [this]() {
    auto key = key_list_->GetSelectedGpgKey();
    if (key == nullptr) return;

    auto* function = new SetOwnerTrustLevel(this);
    function->Exec(key_list_->GetCurrentGpgContextChannel(), key);
    function->deleteLater();
  });
}

void KeyMgmt::create_menus() {
  file_menu_ = menuBar()->addMenu(tr("File"));
  file_menu_->addAction(open_key_file_act_);
  file_menu_->addAction(close_act_);

  key_menu_ = menuBar()->addMenu(tr("Key"));
  generate_key_menu_ = key_menu_->addMenu(tr("Generate Key"));
  generate_key_menu_->addAction(generate_key_pair_act_);
  generate_key_menu_->addAction(generate_subkey_act_);

  import_key_menu_ = key_menu_->addMenu(tr("Import Key"));
  import_key_menu_->addAction(import_key_from_file_act_);
  import_key_menu_->addAction(import_key_from_clipboard_act_);
  import_key_menu_->addAction(import_key_from_key_server_act_);
  import_key_menu_->addAction(import_keys_from_key_package_act_);

  export_key_menu_ = key_menu_->addMenu(tr("Export Key"));
  export_key_menu_->addAction(export_key_to_file_act_);
  export_key_menu_->addAction(export_key_to_clipboard_act_);
  export_key_menu_->addAction(export_key_as_open_ssh_format_);
  key_menu_->addSeparator();
  key_menu_->addAction(delete_checked_keys_act_);
}

void KeyMgmt::create_tool_bars() {
  QToolBar* key_tool_bar = addToolBar(tr("Key"));
  key_tool_bar->setObjectName("keytoolbar");

  // genrate key pair
  key_tool_bar->addAction(generate_key_pair_act_);
  key_tool_bar->addSeparator();

  // add button with popup menu for import
  auto* import_tool_button = new QToolButton(this);
  import_tool_button->setMenu(import_key_menu_);
  import_tool_button->setPopupMode(QToolButton::InstantPopup);
  import_tool_button->setIcon(QIcon(":/icons/key_import.png"));
  import_tool_button->setToolTip(tr("Import key"));
  import_tool_button->setText(tr("Import Key"));
  import_tool_button->setToolButtonStyle(icon_style_);
  key_tool_bar->addWidget(import_tool_button);

  auto* export_tool_button = new QToolButton(this);
  export_tool_button->setMenu(export_key_menu_);
  export_tool_button->setPopupMode(QToolButton::InstantPopup);
  export_tool_button->setIcon(QIcon(":/icons/key_export.png"));
  export_tool_button->setToolTip(tr("Export Key"));
  export_tool_button->setText(tr("Export Key"));
  export_tool_button->setToolButtonStyle(icon_style_);
  key_tool_bar->addWidget(export_tool_button);

  key_tool_bar->addAction(delete_checked_keys_act_);
}

void KeyMgmt::SlotDeleteSelectedKeys() {
  delete_keys_with_warning(key_list_->GetSelectedKeys());
}

void KeyMgmt::SlotDeleteCheckedKeys() {
  delete_keys_with_warning(key_list_->GetCheckedKeys());
}

void KeyMgmt::delete_keys_with_warning(const GpgAbstractKeyPtrList& keys) {
  if (keys.empty()) return;

  QString keynames;
  for (const auto& key : keys) {
    if (!key->IsGood()) continue;
    keynames.append(key->Name());
    keynames.append("<i> &lt;");
    keynames.append(key->Email());
    keynames.append("&gt; </i><br/>");
  }

  int const ret = QMessageBox::warning(
      this, tr("Deleting Keys"),
      "<b>" + tr("Are you sure that you want to delete the following keys?") +
          "</b><br/><br/>" + keynames + +"<br/>" +
          tr("The action can not be undone."),
      QMessageBox::No | QMessageBox::Yes);

  if (ret == QMessageBox::Yes) {
    GpgKeyOpera::GetInstance(key_list_->GetCurrentGpgContextChannel())
        .DeleteKeys(keys);
    emit SignalKeyStatusUpdated();
  }
}

void KeyMgmt::SlotShowKeyDetails() {
  auto keys = key_list_->GetSelectedKeys();
  if (keys.isEmpty()) return;

  CommonUtils::OpenDetailsDialogByKey(
      this, key_list_->GetCurrentGpgContextChannel(), keys.front());
}

void KeyMgmt::SlotExportKeyToKeyPackage() {
  auto keys = key_list_->GetCheckedKeys();
  if (keys.empty()) {
    QMessageBox::critical(
        this, tr("Forbidden"),
        tr("Please check some keys before doing this operation."));
    return;
  }

  auto* dialog = new ExportKeyPackageDialog(
      key_list_->GetCurrentGpgContextChannel(), keys, this);
  dialog->exec();
  emit SignalStatusBarChanged(tr("key(s) exported"));
}

void KeyMgmt::SlotExportKeyToClipboard() {
  auto keys = key_list_->GetCheckedKeys();
  if (keys.empty()) {
    QMessageBox::critical(
        this, tr("Forbidden"),
        tr("Please check some keys before doing this operation."));
    return;
  }

  assert(std::all_of(keys.begin(), keys.end(),
                     [](const auto& key) { return key->IsGood(); }));

  GpgOperaHelper::WaitForOpera(
      this, tr("Exporting"), [=](const OperaWaitingHd& op_hd) {
        GpgKeyImportExporter::GetInstance(
            key_list_->GetCurrentGpgContextChannel())
            .ExportKeys(
                keys, false, true, false, false,
                [=](GpgError err, const DataObjectPtr& data_obj) {
                  // stop waiting
                  op_hd();

                  if (CheckGpgError(err) == GPG_ERR_USER_1) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Unknown error occurred"));
                    return;
                  }

                  if (CheckGpgError(err) != GPG_ERR_NO_ERROR) {
                    CommonUtils::RaiseMessageBox(this, err);
                    return;
                  }

                  if (data_obj == nullptr || !data_obj->Check<GFBuffer>()) {
                    FLOG_W("data object checking failed");
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Unknown error occurred"));
                    return;
                  }

                  auto gf_buffer = ExtractParams<GFBuffer>(data_obj, 0);
                  QApplication::clipboard()->setText(
                      gf_buffer.ConvertToQByteArray());
                });
      });
}

void KeyMgmt::SlotGenerateKeyDialog() {
  (new KeyGenerateDialog(key_list_->GetCurrentGpgContextChannel(), this))
      ->exec();
  this->raise();
}

void KeyMgmt::SlotGenerateSubKey() {
  auto key = key_list_->GetSelectedGpgKey();
  if (key == nullptr) return;

  if (!key->IsPrivateKey()) {
    QMessageBox::critical(this, tr("Invalid Operation"),
                          tr("If a key pair does not have a private key then "
                             "it will not be able to generate sub-keys."));
    return;
  }

  (new SubkeyGenerateDialog(key_list_->GetCurrentGpgContextChannel(), key,
                            this))
      ->exec();
  this->raise();
}

void KeyMgmt::SlotExportAsOpenSSHFormat() {
  auto keys = key_list_->GetCheckedKeys();
  if (keys.empty()) {
    QMessageBox::critical(
        this, tr("Forbidden"),
        tr("Please check a key before performing this operation."));
    return;
  }

  assert(std::all_of(keys.begin(), keys.end(),
                     [](const auto& key) { return key->IsGood(); }));

  if (keys.size() > 1) {
    QMessageBox::critical(this, tr("Forbidden"),
                          tr("This operation accepts just a single key."));
    return;
  }

  GpgOperaHelper::WaitForOpera(
      this, tr("Exporting"), [this, keys](const OperaWaitingHd& op_hd) {
        GpgKeyImportExporter::GetInstance(
            key_list_->GetCurrentGpgContextChannel())
            .ExportKeys(
                keys, false, true, false, true,
                [=](GpgError err, const DataObjectPtr& data_obj) {
                  // stop waiting
                  op_hd();

                  if (CheckGpgError(err) == GPG_ERR_USER_1) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Unknown error occurred"));
                    return;
                  }

                  if (CheckGpgError(err) != GPG_ERR_NO_ERROR) {
                    CommonUtils::RaiseMessageBox(this, err);
                    return;
                  }

                  if (data_obj == nullptr || !data_obj->Check<GFBuffer>()) {
                    QMessageBox::critical(this, tr("Error"),
                                          tr("Unknown error occurred"));
                    return;
                  }

                  auto gf_buffer = ExtractParams<GFBuffer>(data_obj, 0);
                  if (CheckGpgError(err) != GPG_ERR_NO_ERROR) {
                    CommonUtils::RaiseMessageBox(this, err);
                    return;
                  }

                  if (gf_buffer.Empty()) {
                    QMessageBox::critical(
                        this, tr("Error"),
                        tr("This key may not be able to export as OpenSSH "
                           "format. "
                           "Please check the key-size of the subkey(s) used to "
                           "sign."));
                    return;
                  }

                  QString const file_name = QFileDialog::getSaveFileName(
                      this, tr("Export OpenSSH Key To File"), "authorized_keys",
                      tr("OpenSSH Public Key Files") + "All Files (*)");

                  if (!file_name.isEmpty()) {
                    WriteFileGFBuffer(file_name, gf_buffer);
                    emit SignalStatusBarChanged(tr("key(s) exported"));
                  }
                });
      });
}

void KeyMgmt::SlotImportKeyPackage() {
  auto key_package_file_name = QFileDialog::getOpenFileName(
      this, tr("Import Key Package"), {}, tr("Key Package") + " (*.gfepack)");

  if (key_package_file_name.isEmpty()) return;

  // max file size is 32 mb
  QFileInfo key_package_file_info(key_package_file_name);

  if (!key_package_file_info.isFile() || !key_package_file_info.isReadable()) {
    QMessageBox::critical(
        this, tr("Error"),
        tr("Cannot open this file. Please make sure that this "
           "is a regular file and it's readable."));
    return;
  }

  if (key_package_file_info.size() > static_cast<qint64>(32 * 1024 * 1024)) {
    QMessageBox::critical(
        this, tr("Error"),
        tr("The target file is too large for a key package."));
    return;
  }

  auto key_file_name = QFileDialog::getOpenFileName(
      this, tr("Import Key Package Passphrase File"), {},
      tr("Key Package Passphrase File") + " (*.key)");

  if (key_file_name.isEmpty()) return;

  // max file size is 1 mb
  QFileInfo key_file_info(key_file_name);

  if (!key_file_info.isFile() || !key_file_info.isReadable()) {
    QMessageBox::critical(
        this, tr("Error"),
        tr("Cannot open this file. Please make sure that this "
           "is a regular file and it's readable."));
    return;
  }

  if (key_file_info.size() > static_cast<qint64>(1024 * 1024)) {
    QMessageBox::critical(
        this, tr("Error"),
        tr("The target file is too large for a key package passphrase."));
    return;
  }

  GpgOperaHelper::WaitForOpera(
      this, tr("Importing"), [=](const OperaWaitingHd& op_hd) {
        KeyPackageOperator::ImportKeyPackage(
            key_package_file_name, key_file_name,
            key_list_->GetCurrentGpgContextChannel(),
            [=](GFError err, const DataObjectPtr& data_obj) {
              // stop waiting
              op_hd();

              if (err < 0 || !data_obj->Check<GpgImportInformation>()) {
                QMessageBox::critical(
                    this, tr("Error"),
                    tr("An error occur in importing key package."));
                return;
              }

              auto info = ExtractParams<GpgImportInformation>(data_obj, 0);
              if (err >= 0) {
                emit SignalStatusBarChanged(tr("key(s) imported"));
                emit SignalKeyStatusUpdated();

                auto* dialog = new KeyImportDetailDialog(
                    key_list_->GetCurrentGpgContextChannel(),
                    SecureCreateSharedObject<GpgImportInformation>(info), this);
                dialog->exec();
              }
            });
      });
}

}  // namespace GpgFrontend::UI
