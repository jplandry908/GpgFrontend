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

extern "C" {

struct GFGpgSignResult {
  char* signature;
  char* hash_algo;
  char* capsule_id;
  char* error_string;
};

struct GFGpgEncryptionResult {
  char* encrypted_data;
  char* capsule_id;
  char* error_string;
};

struct GFGpgDecryptResult {
  char* decrypted_data;
  char* capsule_id;
  char* error_string;
};

struct GFGpgVerifyResult {
  char* capsule_id;
  char* error_string;
};

struct GFGpgKeyUID {
  char* name;
  char* email;
  char* comment;
};

/**
 * @brief
 *
 * @param key_id
 * @param data
 * @param mode
 * @return const char*
 */
auto GF_SDK_EXPORT GFGpgSignData(int channel, char** key_ids, int key_ids_size,
                                 char* data, int sign_mode, int ascii,
                                 GFGpgSignResult**) -> int;

/**
 * @brief
 *
 * @param channel
 * @param key_ids
 * @param key_ids_size
 * @param data
 * @param ascii
 * @return int
 */
auto GF_SDK_EXPORT GFGpgEncryptData(int channel, char** key_ids,
                                    int key_ids_size, char* data, int ascii,
                                    GFGpgEncryptionResult**) -> int;

/**
 * @brief
 *
 * @param key_id
 * @param data
 * @param mode
 * @return const char*
 */
auto GF_SDK_EXPORT GFGpgDecryptData(int channel, char* data,
                                    GFGpgDecryptResult**) -> int;

/**
 * @brief
 *
 * @param key_id
 * @param data
 * @param mode
 * @return const char*
 */
auto GF_SDK_EXPORT GFGpgVerifyData(int channel, char* data, char* signature,
                                   GFGpgVerifyResult**) -> int;

/**
 * @brief
 *
 * @param key_id
 * @param data
 * @param mode
 * @return const char*
 */
auto GF_SDK_EXPORT GFGpgPublicKey(int channel, char* key_id, int ascii)
    -> char*;

/**
 * @brief
 *
 * @param channel
 * @param key_id
 * @return GpgKeyUID
 */
auto GF_SDK_EXPORT GFGpgKeyPrimaryUID(int channel, char* key_id, GFGpgKeyUID**)
    -> int;
}