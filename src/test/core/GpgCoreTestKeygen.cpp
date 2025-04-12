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

#include "GpgCoreTest.h"
#include "core/function/gpg/GpgKeyGetter.h"
#include "core/function/gpg/GpgKeyOpera.h"
#include "core/model/GpgGenerateKeyResult.h"
#include "core/model/GpgKey.h"
#include "core/model/GpgKeyGenerateInfo.h"
#include "core/utils/GpgUtils.h"
#include "core/utils/MemoryUtils.h"

namespace GpgFrontend::Test {

TEST_F(GpgCoreTest, SearchPrimaryKeyAlgoTest) {
  auto [find, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("rsa2048");
  ASSERT_TRUE(find);
  ASSERT_EQ(algo.Id(), "rsa2048");
  ASSERT_EQ(algo.Id(), "rsa2048");
  ASSERT_EQ(algo.Name(), "RSA");
  ASSERT_EQ(algo.Type(), "RSA");
  ASSERT_EQ(algo.KeyLength(), 2048);
}

TEST_F(GpgCoreTest, SearchSubKeyAlgoTest) {
  auto [find, algo] = KeyGenerateInfo::SearchSubKeyAlgo("rsa2048");
  ASSERT_TRUE(find);
  ASSERT_EQ(algo.Id(), "rsa2048");
  ASSERT_EQ(algo.Name(), "RSA");
  ASSERT_EQ(algo.Type(), "RSA");
  ASSERT_EQ(algo.KeyLength(), 2048);
}

TEST_F(GpgCoreTest, GenerateKeyRSA2048Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_0");
  p_info->SetEmail("bar@gpgfrontend.bktus.com");
  p_info->SetComment("foobar");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("rsa2048");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "rsa2048");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(true);
  p_info->SetNonPassPhrase(true);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);

  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());
  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());

  ASSERT_EQ(key.Name(), "foo_0");
  ASSERT_EQ(key.Email(), "bar@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "foobar");
  ASSERT_EQ(key.PublicKeyAlgo(), "RSA");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 2048);
  ASSERT_EQ(key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_TRUE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_TRUE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeyRSA4096Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_1");
  p_info->SetEmail("bar@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("rsa4096");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "rsa4096");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);

  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.ExpirationTime().date(), expire_time.date());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeyDSA2048Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_1");
  p_info->SetEmail("bar_1@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("dsa2048");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "dsa2048");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);
  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.Name(), "foo_1");
  ASSERT_EQ(key.Email(), "bar_1@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "hello gpgfrontend");
  ASSERT_EQ(key.PublicKeyAlgo(), "DSA");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 2048);
  ASSERT_GT(key.ExpirationTime(), QDateTime::currentDateTime());

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_FALSE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_FALSE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeyED25519Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_4");
  p_info->SetEmail("bar_ed@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("ed25519");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "ed25519");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);
  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.Name(), "foo_4");
  ASSERT_EQ(key.Email(), "bar_ed@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "hello gpgfrontend");
  ASSERT_EQ(key.PublicKeyAlgo(), "EdDSA");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 255);
  ASSERT_GT(key.ExpirationTime(), QDateTime::currentDateTime());

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_FALSE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_FALSE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeyED25519CV25519Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_ec");
  p_info->SetEmail("ec_bar@gpgfrontend.bktus.com");
  p_info->SetComment("ecccc");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("ed25519");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "ed25519");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(true);
  p_info->SetNonPassPhrase(true);

  auto s_info = QSharedPointer<KeyGenerateInfo>::create(true);

  std::tie(found, algo) = KeyGenerateInfo::SearchSubKeyAlgo("cv25519");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "cv25519");
  s_info->SetAlgo(algo);

  s_info->SetNonExpired(true);
  s_info->SetNonPassPhrase(true);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeyWithSubkeySync(p_info, s_info);

  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_TRUE(
      (data_object->Check<GpgGenerateKeyResult, GpgGenerateKeyResult>()));
  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  auto fpr = result.GetFingerprint();
  ASSERT_FALSE(fpr.isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel).GetKey(fpr);
  ASSERT_TRUE(key.IsGood());

  ASSERT_EQ(key.Name(), "foo_ec");
  ASSERT_EQ(key.Email(), "ec_bar@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "ecccc");
  ASSERT_EQ(key.PublicKeyAlgo(), "EdDSA");
  ASSERT_EQ(key.Algo(), "ED25519");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 255);
  ASSERT_EQ(key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_TRUE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_FALSE(key.SubKeys().empty());
  ASSERT_EQ(key.SubKeys().size(), 2);

  auto s_keys = key.SubKeys();
  auto& s_key = s_keys.back();
  ASSERT_EQ(s_key.PublicKeyAlgo(), "ECDH");
  ASSERT_EQ(s_key.Algo(), "CV25519");
  ASSERT_EQ(s_key.KeyLength(), 255);
  ASSERT_EQ(s_key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_FALSE(s_key.IsHasCertCap());
  ASSERT_FALSE(s_key.IsHasAuthCap());
  ASSERT_TRUE(s_key.IsHasEncrCap());
  ASSERT_FALSE(s_key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_TRUE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel).DeleteKey(fpr);
}

TEST_F(GpgCoreTest, GenerateKeyED25519NISTP256Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_ec2");
  p_info->SetEmail("ec2_bar@gpgfrontend.bktus.com");
  p_info->SetComment("ecccc");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("ed25519");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "ed25519");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(true);
  p_info->SetNonPassPhrase(true);

  auto s_info = QSharedPointer<KeyGenerateInfo>::create(true);

  std::tie(found, algo) = KeyGenerateInfo::SearchSubKeyAlgo("nistp256");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "nistp256");
  s_info->SetAlgo(algo);

  s_info->SetNonExpired(true);
  s_info->SetNonPassPhrase(true);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeyWithSubkeySync(p_info, s_info);

  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_TRUE(
      (data_object->Check<GpgGenerateKeyResult, GpgGenerateKeyResult>()));
  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  auto fpr = result.GetFingerprint();
  ASSERT_FALSE(fpr.isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel).GetKey(fpr);
  ASSERT_TRUE(key.IsGood());

  ASSERT_EQ(key.Name(), "foo_ec2");
  ASSERT_EQ(key.Email(), "ec2_bar@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "ecccc");
  ASSERT_EQ(key.PublicKeyAlgo(), "EdDSA");
  ASSERT_EQ(key.Algo(), "ED25519");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 255);
  ASSERT_EQ(key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_TRUE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_FALSE(key.SubKeys().empty());
  ASSERT_EQ(key.SubKeys().size(), 2);

  auto s_keys = key.SubKeys();
  auto& s_key = s_keys.back();
  ASSERT_EQ(s_key.PublicKeyAlgo(), "ECDH");
  ASSERT_EQ(s_key.Algo(), "NISTP256");
  ASSERT_EQ(s_key.KeyLength(), 256);
  ASSERT_EQ(s_key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_FALSE(s_key.IsHasCertCap());
  ASSERT_FALSE(s_key.IsHasAuthCap());
  ASSERT_TRUE(s_key.IsHasEncrCap());
  ASSERT_FALSE(s_key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_TRUE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel).DeleteKey(fpr);
}

TEST_F(GpgCoreTest, GenerateKeyED25519BRAINPOOLP256R1Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_ec3");
  p_info->SetEmail("ec3_bar@gpgfrontend.bktus.com");
  p_info->SetComment("ecccc3");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("ed25519");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "ed25519");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(true);
  p_info->SetNonPassPhrase(true);

  auto s_info = QSharedPointer<KeyGenerateInfo>::create(true);

  std::tie(found, algo) = KeyGenerateInfo::SearchSubKeyAlgo("brainpoolp256r1");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "brainpoolp256r1");
  s_info->SetAlgo(algo);

  s_info->SetNonExpired(true);
  s_info->SetNonPassPhrase(true);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeyWithSubkeySync(p_info, s_info);

  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_TRUE(
      (data_object->Check<GpgGenerateKeyResult, GpgGenerateKeyResult>()));
  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  auto fpr = result.GetFingerprint();
  ASSERT_FALSE(fpr.isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel).GetKey(fpr);
  ASSERT_TRUE(key.IsGood());

  ASSERT_EQ(key.Name(), "foo_ec3");
  ASSERT_EQ(key.Email(), "ec3_bar@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "ecccc3");
  ASSERT_EQ(key.PublicKeyAlgo(), "EdDSA");
  ASSERT_EQ(key.Algo(), "ED25519");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 255);
  ASSERT_EQ(key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_TRUE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_FALSE(key.SubKeys().empty());
  ASSERT_EQ(key.SubKeys().size(), 2);

  auto s_keys = key.SubKeys();
  auto& s_key = s_keys.back();
  ASSERT_EQ(s_key.PublicKeyAlgo(), "ECDH");
  ASSERT_EQ(s_key.Algo(), "BRAINPOOLP256R1");
  ASSERT_EQ(s_key.KeyLength(), 256);
  ASSERT_EQ(s_key.ExpirationTime(), QDateTime::fromMSecsSinceEpoch(0));

  ASSERT_FALSE(s_key.IsHasCertCap());
  ASSERT_FALSE(s_key.IsHasAuthCap());
  ASSERT_TRUE(s_key.IsHasEncrCap());
  ASSERT_FALSE(s_key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_TRUE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel).DeleteKey(fpr);
}

TEST_F(GpgCoreTest, GenerateKeyNISTP256Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_4");
  p_info->SetEmail("bar_ed@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("nistp256");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "nistp256");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);
  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.Name(), "foo_4");
  ASSERT_EQ(key.Email(), "bar_ed@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "hello gpgfrontend");
  ASSERT_EQ(key.PublicKeyAlgo(), "ECDSA");
  ASSERT_EQ(key.Algo(), "NISTP256");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 256);
  ASSERT_GT(key.ExpirationTime(), QDateTime::currentDateTime());

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_FALSE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_FALSE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeyED448Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_4");
  p_info->SetEmail("bar_ed@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("ed448");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "ed448");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);
  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.Name(), "foo_4");
  ASSERT_EQ(key.Email(), "bar_ed@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "hello gpgfrontend");
  ASSERT_EQ(key.PublicKeyAlgo(), "EdDSA");
  ASSERT_EQ(key.Algo(), "ED448");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 448);
  ASSERT_GT(key.ExpirationTime(), QDateTime::currentDateTime());

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_FALSE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_FALSE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

TEST_F(GpgCoreTest, GenerateKeySECP256K1Test) {
  auto p_info = QSharedPointer<KeyGenerateInfo>::create();
  p_info->SetName("foo_4");
  p_info->SetEmail("bar_ed@gpgfrontend.bktus.com");
  p_info->SetComment("hello gpgfrontend");

  auto [found, algo] = KeyGenerateInfo::SearchPrimaryKeyAlgo("secp256k1");
  ASSERT_TRUE(found);
  ASSERT_EQ(algo.Id(), "secp256k1");
  p_info->SetAlgo(algo);

  p_info->SetNonExpired(false);
  p_info->SetNonPassPhrase(true);

  auto expire_time =
      QDateTime::currentDateTime().addSecs(static_cast<qint64>(24 * 3600));
  p_info->SetExpireTime(expire_time);
  p_info->SetNonPassPhrase(false);

  auto [err, data_object] = GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
                                .GenerateKeySync(p_info);
  ASSERT_EQ(CheckGpgError(err), GPG_ERR_NO_ERROR);
  ASSERT_EQ(data_object->GetObjectSize(), 1);
  ASSERT_TRUE(data_object->Check<GpgGenerateKeyResult>());

  auto result = ExtractParams<GpgGenerateKeyResult>(data_object, 0);
  ASSERT_TRUE(result.IsGood());
  ASSERT_FALSE(result.GetFingerprint().isEmpty());

  auto key = GpgKeyGetter::GetInstance(kGpgFrontendDefaultChannel)
                 .GetKey(result.GetFingerprint());
  ASSERT_TRUE(key.IsGood());
  ASSERT_EQ(key.Name(), "foo_4");
  ASSERT_EQ(key.Email(), "bar_ed@gpgfrontend.bktus.com");
  ASSERT_EQ(key.Comment(), "hello gpgfrontend");
  ASSERT_EQ(key.PublicKeyAlgo(), "ECDSA");
  ASSERT_EQ(key.Algo(), "SECP256K1");
  ASSERT_EQ(key.OwnerTrustLevel(), 5);
  ASSERT_EQ(key.PrimaryKeyLength(), 256);
  ASSERT_GT(key.ExpirationTime(), QDateTime::currentDateTime());

  ASSERT_TRUE(key.IsHasCertCap());
  ASSERT_TRUE(key.IsHasAuthCap());
  ASSERT_FALSE(key.IsHasEncrCap());
  ASSERT_TRUE(key.IsHasSignCap());

  ASSERT_TRUE(key.IsHasActualCertCap());
  ASSERT_TRUE(key.IsHasActualAuthCap());
  ASSERT_FALSE(key.IsHasActualEncrCap());
  ASSERT_TRUE(key.IsHasActualSignCap());

  GpgKeyOpera::GetInstance(kGpgFrontendDefaultChannel)
      .DeleteKey(result.GetFingerprint());
}

}  // namespace GpgFrontend::Test
