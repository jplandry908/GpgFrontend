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

#include "core/module/Event.h"
#include "core/typedef/CoreTypedef.h"
#include "core/typedef/GpgTypedef.h"
#include "ui/main_window/GeneralMainWindow.h"
#include "ui/struct/GpgOperaResult.h"

namespace GpgFrontend {
class GpgPassphraseContext;
class GpgResultAnalyse;
class GpgVerifyResultAnalyse;
}  // namespace GpgFrontend

namespace GpgFrontend::UI {

class KeyList;
class TextEdit;
class InfoBoardWidget;
struct GpgOperaContext;
struct GpgOperaContextBasement;
struct KeyTable;

/**
 * @brief
 *
 */
class MainWindow : public GeneralMainWindow {
  Q_OBJECT

 public:
  /**
   *
   */
  struct OperationMenu {
    using OperationType = unsigned int;

    static constexpr OperationType kNone = 0;
    static constexpr OperationType kEncrypt = 1 << 0;
    static constexpr OperationType kSign = 1 << 1;
    static constexpr OperationType kDecrypt = 1 << 2;
    static constexpr OperationType kVerify = 1 << 3;
    static constexpr OperationType kEncryptAndSign = 1 << 4;
    static constexpr OperationType kDecryptAndVerify = 1 << 5;
    static constexpr OperationType kSymmetricEncrypt = 1 << 6;
  };

  /**
   * @brief
   *
   */
  MainWindow();

  /**
   * @details ONLY Called from main()
   */
  void Init() noexcept;

 signals:

  /**
   * @brief
   */
  void SignalLoaded();

  /**
   * @brief
   */
  void SignalRestartApplication(int);

  /**
   * @brief
   */
  void SignalUIRefresh();

  /**
   * @brief
   */
  void SignalKeyDatabaseRefresh();

 public slots:

  /**
   * @brief
   */
  void SlotSetStatusBarText(const QString& text);

 protected:
  /**
   * @details Close event shows a save dialog, if there are unsaved documents on
   * exit.
   * @param event
   */
  void closeEvent(QCloseEvent* event) override;

 public slots:

  /**
   * @details Open a new tab for path
   */
  void SlotOpenFile(const QString& path);

  /**
   * @details encrypt the text of currently active textedit-page
   * with the currently checked keys
   */
  void SlotEncrypt();

  /**
   * @details encrypt and sign the text of currently active textedit-page
   * with the currently checked keys
   */
  void SlotEncryptSign();

  /**
   * @details Show a passphrase dialog and decrypt the text of currently active
   * tab.
   */
  void SlotDecrypt();

  /**
   * @details Sign the text of currently active tab with the checked private
   * keys
   */
  void SlotSign();

  /**
   * @details Verify the text of currently active tab and show verify
   * information. If document is signed with a key, which is not in keylist,
   * show import missing key from keyserver in Menu of verifynotification.
   */
  void SlotVerify();

  /**
   * @brief
   *
   */
  void SlotVerifyEML();

  /**
   * @brief
   *
   */
  void SlotDecryptEML();

  /**
   * @brief
   *
   */
  void SlotSignEML();

  /**
   * @brief
   *
   */
  void SlotEncryptEML();

  /**
   * @brief
   *
   */
  void SlotEncryptSignEML();

  /**
   * @brief
   *
   */
  void SlotDecryptVerifyEML();

  /**
   * @details decrypt and verify the text of currently active textedit-page
   * with the currently checked keys
   */
  void SlotDecryptVerify();

  /**
   * @details Open dialog for encrypting file.
   */
  void SlotFileEncrypt(const QStringList& paths, bool ascii);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileDecrypt(const QStringList& paths);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileSign(const QStringList& paths, bool ascii);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileVerify(const QStringList& paths);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileVerifyEML(const QString& path);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileEncryptSign(const QStringList& paths, bool ascii);

  /**
   * @brief
   *
   * @param path
   */
  void SlotFileDecryptVerify(const QStringList& paths);

  /**
   * @details get value of member restartNeeded to needed.
   * @param needed true, if application has to be restarted
   */
  void SlotSetRestartNeeded(int);

  /**
   * @brief
   *
   */
  void SlotGeneralEncrypt(bool);

  /**
   * @brief
   *
   */
  void SlotGeneralDecrypt(bool);

  /**
   * @brief
   *
   */
  void SlotGeneralSign(bool);

  /**
   * @brief
   *
   */
  void SlotGeneralVerify(bool);

  /**
   * @brief
   *
   */
  void SlotGeneralEncryptSign(bool);

  /**
   * @brief
   *
   */
  void SlotGeneralDecryptVerify(bool);

 private slots:

  /**
   * @brief
   *
   */
  void slot_refresh_current_file_view();

  /**
   * @details Show the details of the first of the first of selected keys
   */
  void slot_show_key_details();

  /**
   * @details Refresh key information of selected keys from default keyserver
   */
  void refresh_keys_from_key_server();

  /**
   * @details upload the selected key to the keyserver
   */
  void upload_key_to_server();

  /**
   * @details Open find widget.
   */
  void slot_find();

  /**
   * @details start the wizard
   */
  void slot_start_wizard();

  /**
   * @details Import keys from currently active tab to keylist if possible.
   */
  void slot_import_key_from_edit();

  /**
   * @details Append the selected keys to currently active textedit.
   */
  void slot_append_selected_keys();

  /**
   * @brief
   *
   */
  void slot_append_keys_create_datetime();

  /**
   * @brief
   *
   */
  void slot_append_keys_expire_datetime();

  /**
   * @brief
   *
   */
  void slot_append_keys_fingerprint();

  /**
   * @details Copy the mailaddress of selected key to clipboard.
   * Method for keylists contextmenu.
   */
  void slot_copy_mail_address_to_clipboard();

  /**
   * @details Copy the mailaddress of selected key to clipboard.
   * Method for keylists contextmenu.
   */
  void slot_copy_default_uid_to_clipboard();

  /**
   * @details Copy the mailaddress of selected key to clipboard.
   * Method for keylists contextmenu.
   */
  void slot_copy_key_id_to_clipboard();

  /**
   * @details Open key management dialog.
   */
  void slot_open_key_management();

  /**
   * @brief
   *
   */
  void slot_default_file_tab();

  /**
   * @details Open File Opera Tab
   */
  void slot_open_file_tab();

  /**
   * @details Open settings-dialog.
   */
  void slot_open_settings_dialog();

  /**
   * @details Replace double linebreaks by single linebreaks in currently active
   * tab.
   */
  void slot_clean_double_line_breaks();

  /**
   * @details Cut the existing PGP header and footer from current tab.
   */
  void slot_cut_pgp_header();

  /**
   * @details Add PGP header and footer to current tab.
   */
  void slot_add_pgp_header();

  /**
   * @details Disable tab related actions, if number of tabs is 0.
   * @param number number of the opened tabs and -1, if no tab is opened
   */
  void slot_switch_menu_control_mode(int);

  /**
   * @details called when need to upgrade.
   */
  void slot_version_upgrade_notify();

  /**
   * @details
   */
  void slot_add_key_2_favorite();

  /**
   * @details
   */
  void slot_remove_key_from_favorite();

  /**
   * @details
   */
  void slot_set_owner_trust_level_of_key();

  /**
   * @brief
   *
   */
  void slot_verify_email_by_eml_data(const QByteArray& buffer);

  /**
   * @brief
   *
   * @param buffer
   */
  void slot_verify_email_by_eml_data_result_helper(
      const Module::Event::Params& p);

  /**
   * @brief
   *
   * @param result_analyse
   */
  void slot_verifying_unknown_signature_helper(const QStringList& fprs);

  /**
   * @brief
   *
   * @param result_analyse
   */
  void slot_result_analyse_show_helper(const GpgResultAnalyse& r);

  /**
   * @brief
   *
   * @param r_a
   * @param r_b
   */
  void slot_result_analyse_show_helper(const GpgResultAnalyse& r_a,
                                       const GpgResultAnalyse& r_b);

  /**
   * @brief
   *
   * @param opera_results
   */
  void slot_result_analyse_show_helper(
      const QContainer<GpgOperaResult>& opera_results);

  /**
   * @brief
   *
   * @param result_analyse
   */
  void slot_eml_verify_show_helper(const QString& email_info,
                                   const GpgVerifyResultAnalyse& r);

  /**
   * @brief
   *
   * @param status
   * @param text
   */
  void slot_refresh_info_board(int status, const QString& text);

  /**
   * @brief
   *
   */
  void slot_clean_gpg_password_cache(bool);

  /**
   * @brief
   *
   */
  void slot_reload_gpg_components(bool);

  /**
   * @brief
   *
   */
  void slot_restart_gpg_components(bool);

  /**
   * @brief
   *
   * @param buffer
   */
  void slot_decrypt_email_by_eml_data(const QByteArray& buffer);

  /**
   * @brief
   *
   * @param results
   */
  void slot_gpg_opera_buffer_show_helper(
      const QContainer<GpgOperaResult>& results);

  /**
   * @details refresh and enable specify crypto-menu actions.
   */
  void slot_update_crypto_operations_menu(unsigned int mask);

  /**
   * @brief
   *
   * @param results
   */
  void slot_update_operations_menu_by_checked_keys(unsigned int mask);

  /**
   * @brief
   *
   * @param event
   */
  void slot_popup_menu_by_key_list(QContextMenuEvent* event, KeyTable*);

  /**
   * @brief
   *
   */
  void slot_open_file_tab_with_directory();

 private:
  /**
   * @details Create actions for the main-menu and the context-menu of the
   * keylist.
   */
  void create_actions();

  /**
   * @details create the menu of the main-window.
   */
  void create_menus();

  /**
   * @details Create edit-, crypt- and key-toolbars.
   */
  void create_tool_bars();

  /**
   * @details Create statusbar of mainwindow.
   */
  void create_status_bar();

  /**
   * @details Create keylist- and attachment-dockwindows.
   */
  void create_dock_windows();

  /**
   * @details Create attachment dock window.
   */
  void create_attachment_dock();

  /**
   * @details close attachment-dockwindow.
   */
  void close_attachment_dock();

  /**
   * @details Load settings from ini-file.
   */
  void restore_settings();

  /**
   * @brief Create a action object
   *
   * @param id
   * @param name
   * @param icon
   * @param too_tip
   * @return QAction*
   */
  auto create_action(const QString& id, const QString& name,
                     const QString& icon, const QString& too_tip,
                     const QContainer<QKeySequence>& shortcuts = {})
      -> QAction*;

  /**
   * @brief
   *
   */
  void decrypt_email_by_eml_data_result_helper(Module::Event::Params p);

  /**
   * @brief
   *
   * @param err_code
   * @param error_string
   * @return QString
   */
  auto handle_module_error(Module::Event::Params p) -> bool;

  /**
   * @brief
   *
   * @param paths
   * @return true
   * @return false
   */
  auto check_read_file_paths_helper(const QStringList& paths) -> bool;

  /**
   * @brief
   *
   * @param paths
   * @return true
   * @return false
   */
  auto check_write_file_paths_helper(const QStringList& paths) -> bool;

  /**
   * @brief
   *
   * @param key_ids
   * @param capability_check
   * @param capability_err_string
   * @return GpgKeyList
   */
  auto check_keys_helper(
      const GpgAbstractKeyPtrList& keys,
      const std::function<bool(const GpgAbstractKeyPtr&)>& capability_check,
      const QString& capability_err_string) -> GpgAbstractKeyPtrList;

  /**
   * @brief
   *
   * @param context
   * @return auto
   */
  void exec_operas_helper(
      const QString& task,
      const QSharedPointer<GpgOperaContextBasement>& contexts);

  /**
   * @brief
   *
   * @param contexts
   * @param key_ids
   * @param keys
   * @return true
   * @return false
   */
  auto encrypt_operation_key_validate(
      const QSharedPointer<GpgOperaContextBasement>& contexts) -> bool;

  /**
   * @brief
   *
   * @param contexts
   * @return true
   * @return false
   */
  auto sign_operation_key_validate(
      const QSharedPointer<GpgOperaContextBasement>& contexts) -> bool;

  /**
   * @brief
   *
   * @param contexts
   * @return true
   * @return false
   */
  auto fuzzy_signature_key_elimination(
      const QSharedPointer<GpgOperaContextBasement>& contexts) -> bool;

  /**
   * @brief
   *
   * @return auto
   */
  void check_update_at_startup();

  TextEdit* edit_{};          ///< Tabwidget holding the edit-windows
  QMenu* file_menu_{};        ///<  Submenu for file-operations
  QMenu* edit_menu_{};        ///<  Submenu for text-operations
  QMenu* crypt_menu_{};       ///<  Submenu for crypt-operations
  QMenu* advance_menu_{};     ///<  Submenu for help-operations
  QMenu* help_menu_{};        ///<  Submenu for help-operations
  QMenu* key_menu_{};         ///<  Submenu for key-operations
  QMenu* view_menu_{};        ///<  Submenu for view operations
  QMenu* import_key_menu_{};  ///<  Submenu for import operations
  QMenu* workspace_menu_{};   ///<
  QMenu* open_menu_{};

  QToolBar* crypt_tool_bar_{};  ///<  Toolbar holding crypt actions
  QToolBar* file_tool_bar_{};   ///<  Toolbar holding file actions
  QToolBar* edit_tool_bar_{};   ///<  Toolbar holding edit actions
  QToolBar*
      special_edit_tool_bar_{};  ///<  Toolbar holding special edit actions
  QToolBar* key_tool_bar_{};     ///<  Toolbar holding key operations
  QToolButton*
      import_button_{};  ///<  Tool button for import dropdown menu in toolbar
  QToolButton* open_button_{};       ///<
  QToolButton* workspace_button_{};  ///<
  QDockWidget* key_list_dock_{};     ///<  Encrypt Dock
  QDockWidget* attachment_dock_{};   ///<  Attachment Dock
  QDockWidget* info_board_dock_{};

  QAction* new_tab_act_{};               ///<  Action to create new tab
  QAction* new_email_tab_act_{};         ///<  Action to create email tab
  QAction* switch_tab_up_act_{};         ///<  Action to switch tab up
  QAction* switch_tab_down_act_{};       ///<  Action to switch tab down
  QAction* browser_file_act_{};          ///<  Action to open file browser
  QAction* browser_dir_act_{};           ///<  Action to open file browser
  QAction* browser_act_{};               ///<  Action to open file browser
  QAction* save_act_{};                  ///<  Action to save file
  QAction* save_as_act_{};               ///<  Action to save file as
  QAction* print_act_{};                 ///<  Action to print
  QAction* close_tab_act_{};             ///<  Action to print
  QAction* quit_act_{};                  ///<  Action to quit application
  QAction* sym_encrypt_act_{};           ///<  Action to symmetric encrypt text
  QAction* encrypt_act_{};               ///<  Action to encrypt text
  QAction* encrypt_sign_act_{};          ///<  Action to encrypt and sign text
  QAction* decrypt_verify_act_{};        ///<  Action to encrypt and sign text
  QAction* decrypt_act_{};               ///<  Action to decrypt text
  QAction* sign_act_{};                  ///<  Action to sign text
  QAction* verify_act_{};                ///<  Action to verify text
  QAction* import_key_from_edit_act_{};  ///<  Action to import key from edit
  QAction* clean_double_line_breaks_act_{};  ///<  Action to remove double
  QAction* generate_key_pair_act_{};

  QAction* gnupg_controller_open_act_{};       ///<
  QAction* module_controller_open_act_{};      ///<
  QAction* smart_card_controller_open_act_{};  ///<

  QAction* clean_gpg_password_cache_act_{};  ///<
  QAction* reload_components_act_{};         ///<
  QAction* restart_components_act_{};        ///<

  QAction* append_selected_keys_act_{};  ///< Action to append selected keys
                                         ///< to edit
  QAction* append_key_fingerprint_to_editor_act_{};  ///<
  QAction* append_key_create_date_to_editor_act_{};  ///<
  QAction* append_key_expire_date_to_editor_act_{};  ///<

  QAction* copy_mail_address_to_clipboard_act_{};  ///< Action to copy mail to
                                                   ///< clipboard
  QAction* copy_key_id_to_clipboard_act_{};        ///<
  QAction* copy_key_default_uid_to_clipboard_act_{};  ///<

  QAction* add_key_2_favourite_act_{};        ///<
  QAction* remove_key_from_favourtie_act_{};  ///<
  QAction* set_owner_trust_of_key_act_{};     ///<

  QAction* open_key_management_act_{};   ///< Action to open key management
  QAction* copy_act_{};                  ///< Action to copy text
  QAction* quote_act_{};                 ///< Action to quote text
  QAction* cut_act_{};                   ///< Action to cut text
  QAction* paste_act_{};                 ///< Action to paste text
  QAction* select_all_act_{};            ///< Action to select whole text
  QAction* find_act_{};                  ///< Action to find text
  QAction* undo_act_{};                  ///< Action to undo last action
  QAction* redo_act_{};                  ///< Action to redo last action
  QAction* zoom_in_act_{};               ///< Action to zoom in
  QAction* zoom_out_act_{};              ///< Action to zoom out
  QAction* about_act_{};                 ///< Action to open about dialog
  QAction* check_update_act_{};          ///< Action to open about dialog
  QAction* translate_act_{};             ///< Action to open about dialog
  QAction* gnupg_act_{};                 ///< Action to open about dialog
  QAction* open_settings_act_{};         ///< Action to open settings dialog
  QAction* show_key_details_act_{};      ///< Action to open key-details dialog
  QAction* start_wizard_act_{};          ///< Action to open the wizard
  QAction* cut_pgp_header_act_{};        ///< Action for cutting the PGP header
  QAction* add_pgp_header_act_{};        ///< Action for adding the PGP header
  QAction* import_key_from_file_act_{};  ///<
  QAction* import_key_from_clipboard_act_{};   ///<
  QAction* import_key_from_key_server_act_{};  ///<

  QLabel* status_bar_icon_{};  ///<

  KeyList* m_key_list_{};          ///<
  InfoBoardWidget* info_board_{};  ///<
  QMap<QString, QPointer<QAction>> buffered_actions_;

  QMenu* popup_menu_;

  bool attachment_dock_created_{};         ///<
  int restart_mode_{0};                    ///<
  bool prohibit_update_checking_ = false;  ///<
  unsigned int operations_menu_mask_ = ~0;
};

}  // namespace GpgFrontend::UI
