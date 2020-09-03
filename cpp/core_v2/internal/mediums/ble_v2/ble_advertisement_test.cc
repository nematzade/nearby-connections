// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "core_v2/internal/mediums/ble_v2/ble_advertisement.h"

#include <algorithm>

#include "gtest/gtest.h"

namespace location {
namespace nearby {
namespace connections {
namespace mediums {
namespace {

constexpr BleAdvertisement::Version kVersion = BleAdvertisement::Version::kV2;
constexpr BleAdvertisement::SocketVersion kSocketVersion =
    BleAdvertisement::SocketVersion::kV2;
constexpr absl::string_view kServiceIDHashBytes{"\x0a\x0b\x0c"};
constexpr absl::string_view kData{
    "How much wood can a woodchuck chuck if a wood chuck would chuck wood?"};
constexpr absl::string_view kFastData{"Fast Advertise"};
constexpr absl::string_view kDeviceToken{"\x04\x20"};
// kAdvertisementLength/kFastAdvertisementLength corresponds to the length of a
// specific BleAdvertisement packed with the kData/kFastData given above. Be
// sure to update this if kData/kFastData ever changes.
constexpr size_t kAdvertisementLength = 77;
constexpr size_t kFastAdvertisementLength = 16;
constexpr size_t kLongAdvertisementLength = kAdvertisementLength + 1000;

TEST(BleAdvertisementTest, ConstructionWorksV1) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{BleAdvertisement::Version::kV1,
                                     BleAdvertisement::SocketVersion::kV1,
                                     service_id_hash,
                                     data,
                                     device_token};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_FALSE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(BleAdvertisement::Version::kV1, ble_advertisement.GetVersion());
  EXPECT_EQ(BleAdvertisement::SocketVersion::kV1,
            ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest, ConstructionWorksV1ForFastAdvertisement) {
  ByteArray fast_data{std::string(kFastData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{BleAdvertisement::Version::kV1,
                                     BleAdvertisement::SocketVersion::kV1,
                                     fast_data,
                                     device_token};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_TRUE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(BleAdvertisement::Version::kV1, ble_advertisement.GetVersion());
  EXPECT_EQ(BleAdvertisement::SocketVersion::kV1,
            ble_advertisement.GetSocketVersion());
  EXPECT_EQ(fast_data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(fast_data, ble_advertisement.GetData());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest, ConstructionFailsWithBadVersion) {
  BleAdvertisement::Version bad_version =
      static_cast<BleAdvertisement::Version>(666);

  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{bad_version,
                                     kSocketVersion,
                                     service_id_hash,
                                     data,
                                     device_token};
  EXPECT_FALSE(ble_advertisement.IsValid());

  BleAdvertisement fast_ble_advertisement{bad_version,
                                          kSocketVersion,
                                          data,
                                          device_token};
  EXPECT_FALSE(fast_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithBadSocketVersion) {
  BleAdvertisement::SocketVersion bad_socket_version =
      static_cast<BleAdvertisement::SocketVersion>(666);

  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{kVersion,
                                     bad_socket_version,
                                     service_id_hash,
                                     data,
                                     device_token};
  EXPECT_FALSE(ble_advertisement.IsValid());

  BleAdvertisement fast_ble_advertisement{kVersion,
                                          bad_socket_version,
                                          data,
                                          device_token};
  EXPECT_FALSE(fast_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithShortServiceIdHash) {
  char short_service_id_hash_bytes[] = "\x0a\x0b";

  ByteArray bad_service_id_hash{short_service_id_hash_bytes};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{kVersion,
                                     kSocketVersion,
                                     bad_service_id_hash,
                                     data,
                                     device_token};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithLongServiceIdHash) {
  char long_service_id_hash_bytes[] = "\x0a\x0b\x0c\x0d";

  ByteArray bad_service_id_hash{long_service_id_hash_bytes};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{kVersion,
                                     kSocketVersion,
                                     bad_service_id_hash,
                                     data,
                                     device_token};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFailsWithLongData) {
  // BleAdvertisement shouldn't be able to support data with the max GATT
  // attribute length because it needs some room for the preceding fields.
  char long_data[512]{};

  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray bad_data{long_data, 512};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement ble_advertisement{kVersion,
                                     kSocketVersion,
                                     service_id_hash,
                                     bad_data,
                                     device_token};
  EXPECT_FALSE(ble_advertisement.IsValid());

  BleAdvertisement fast_ble_advertisement{kVersion,
                                          kSocketVersion,
                                          bad_data,
                                          device_token};
  EXPECT_FALSE(fast_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionWorksWithEmptyDeviceToken) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};

  BleAdvertisement ble_advertisement{kVersion,
                                     kSocketVersion,
                                     service_id_hash,
                                     data,
                                     ByteArray{}};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_FALSE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
  EXPECT_TRUE(ble_advertisement.GetDeviceToken().Empty());
}

TEST(BleAdvertisementTest,
     ConstructionWorksWithEmptyDeviceTokenForFastAdvertisement) {
  ByteArray fast_data{std::string(kFastData)};

  BleAdvertisement ble_advertisement{kVersion,
                                     kSocketVersion,
                                     fast_data,
                                     ByteArray{}};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_TRUE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(fast_data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(fast_data, ble_advertisement.GetData());
  EXPECT_TRUE(ble_advertisement.GetDeviceToken().Empty());
}

TEST(BleAdvertisementTest, ConstructionFailsWithWrongSizeofDeviceToken) {
  char wrong_device_token_bytes_1[] = "\x04\x2\x10";  // over 2 bytes
  char wrong_device_token_bytes_2[] = "\x04";         // 1 byte

  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray bad_device_token_1{wrong_device_token_bytes_1};
  ByteArray bad_device_token_2{wrong_device_token_bytes_2};

  BleAdvertisement ble_advertisement_1{kVersion,
                                       kSocketVersion,
                                       service_id_hash,
                                       data,
                                       bad_device_token_1};
  EXPECT_FALSE(ble_advertisement_1.IsValid());

  BleAdvertisement ble_advertisement_2{kVersion,
                                       kSocketVersion,
                                       service_id_hash,
                                       data,
                                       bad_device_token_2};
  EXPECT_FALSE(ble_advertisement_2.IsValid());

  BleAdvertisement fast_ble_advertisement_1{kVersion,
                                            kSocketVersion,
                                            data,
                                            bad_device_token_1};
  EXPECT_FALSE(fast_ble_advertisement_1.IsValid());

  BleAdvertisement fast_ble_advertisement_2{kVersion,
                                            kSocketVersion,
                                            data,
                                            bad_device_token_2};
  EXPECT_FALSE(fast_ble_advertisement_2.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFromSerializedBytesWorks) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         service_id_hash,
                                         data,
                                         device_token};

  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_FALSE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(data, ble_advertisement.GetData());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest,
     ConstructionFromSerializedBytesWorksForAdvertisement) {
  ByteArray fast_data{std::string(kFastData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         fast_data,
                                         device_token};

  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_TRUE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(fast_data.size(), ble_advertisement.GetData().size());
  EXPECT_EQ(fast_data, ble_advertisement.GetData());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest, ConstructionFromSerializedBytesWithEmptyDataWorks) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         service_id_hash,
                                         ByteArray(),
                                         device_token};
  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_FALSE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, ble_advertisement.GetServiceIdHash());
  EXPECT_TRUE(ble_advertisement.GetData().Empty());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest,
     ConstructionFromSerializedBytesWithEmptyDataWorksForFastAdvertisement) {
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         ByteArray(),
                                         device_token};
  ByteArray ble_advertisement_bytes{org_ble_advertisement};
  BleAdvertisement ble_advertisement{ble_advertisement_bytes};

  EXPECT_TRUE(ble_advertisement.IsValid());
  EXPECT_TRUE(ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, ble_advertisement.GetSocketVersion());
  EXPECT_TRUE(ble_advertisement.GetData().Empty());
  EXPECT_EQ(device_token, ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest, ConstructionFromExtraSerializedBytesWorks) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         service_id_hash,
                                         data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Copy the bytes into a new array with extra bytes. We must explicitly
  // define how long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kLongAdvertisementLength]{};
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         std::min(sizeof(raw_ble_advertisement_bytes),
                  org_ble_advertisement_bytes.size()));

  // Re-parse the Ble advertisement using our extra long advertisement bytes.
  ByteArray long_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                         kLongAdvertisementLength};
  BleAdvertisement long_ble_advertisement{long_ble_advertisement_bytes};

  EXPECT_TRUE(long_ble_advertisement.IsValid());
  EXPECT_FALSE(long_ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, long_ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, long_ble_advertisement.GetSocketVersion());
  EXPECT_EQ(service_id_hash, long_ble_advertisement.GetServiceIdHash());
  EXPECT_EQ(data.size(), long_ble_advertisement.GetData().size());
  EXPECT_EQ(data, long_ble_advertisement.GetData());
  EXPECT_EQ(device_token, long_ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest,
     ConstructionFromExtraSerializedBytesWorksForFastAdvertisement) {
  ByteArray fast_data{std::string(kFastData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         fast_data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Copy the bytes into a new array with extra bytes. We must explicitly
  // define how long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kLongAdvertisementLength]{};
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         std::min(sizeof(raw_ble_advertisement_bytes),
                  org_ble_advertisement_bytes.size()));

  // Re-parse the Ble advertisement using our extra long advertisement bytes.
  ByteArray long_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                         kLongAdvertisementLength};
  BleAdvertisement long_ble_advertisement{long_ble_advertisement_bytes};

  EXPECT_TRUE(long_ble_advertisement.IsValid());
  EXPECT_TRUE(long_ble_advertisement.IsFastAdvertisement());
  EXPECT_EQ(kVersion, long_ble_advertisement.GetVersion());
  EXPECT_EQ(kSocketVersion, long_ble_advertisement.GetSocketVersion());
  EXPECT_EQ(fast_data.size(), long_ble_advertisement.GetData().size());
  EXPECT_EQ(fast_data, long_ble_advertisement.GetData());
  EXPECT_EQ(device_token, long_ble_advertisement.GetDeviceToken());
}

TEST(BleAdvertisementTest, ConstructionFromNullBytesFails) {
  BleAdvertisement ble_advertisement{ByteArray{}};

  EXPECT_FALSE(ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest, ConstructionFromShortLengthSerializedBytesFails) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         service_id_hash,
                                         data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Cut off the advertisement so that it's too short.
  ByteArray short_ble_advertisement_bytes{org_ble_advertisement_bytes.data(),
                                          7};
  BleAdvertisement short_ble_advertisement{short_ble_advertisement_bytes};

  EXPECT_FALSE(short_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest,
     ConstructionFromShortLengthSerializedBytesFailsForFastAdvertisement) {
  ByteArray fast_data{std::string(kFastData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         fast_data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Cut off the advertisement so that it's too short.
  ByteArray short_ble_advertisement_bytes{org_ble_advertisement_bytes.data(),
                                          2};
  BleAdvertisement short_ble_advertisement{short_ble_advertisement_bytes};

  EXPECT_FALSE(short_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest,
     ConstructionFromSerializedBytesWithInvalidDataLengthFails) {
  ByteArray service_id_hash{std::string(kServiceIDHashBytes)};
  ByteArray data{std::string(kData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         service_id_hash,
                                         data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Corrupt the DATA_SIZE bits. Start by making a raw copy of the Ble
  // advertisement bytes so we can modify it. We must explicitly define how
  // long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kAdvertisementLength];
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         kAdvertisementLength);

  // The data size field lives in indices 4-7. Corrupt it.
  memset(raw_ble_advertisement_bytes + 4, 0xFF, 4);

  // Try to parse the Ble advertisement using our corrupted advertisement bytes.
  ByteArray corrupted_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                              kAdvertisementLength};
  BleAdvertisement corrupted_ble_advertisement{
      corrupted_ble_advertisement_bytes};

  EXPECT_FALSE(corrupted_ble_advertisement.IsValid());
}

TEST(BleAdvertisementTest,
     ConstructionFromSerializedBytesWithInvalidDataLengthFails2) {
  ByteArray fast_data{std::string(kFastData)};
  ByteArray device_token{std::string(kDeviceToken)};

  BleAdvertisement org_ble_advertisement{kVersion,
                                         kSocketVersion,
                                         fast_data,
                                         device_token};
  ByteArray org_ble_advertisement_bytes{org_ble_advertisement};

  // Corrupt the DATA_SIZE bits. Start by making a raw copy of the Ble
  // advertisement bytes so we can modify it. We must explicitly define how
  // long our array is because we can't use variable length arrays.
  char raw_ble_advertisement_bytes[kFastAdvertisementLength];
  memcpy(raw_ble_advertisement_bytes, org_ble_advertisement_bytes.data(),
         kFastAdvertisementLength);

  // The data size field lives in index 1. Corrupt it.
  memset(raw_ble_advertisement_bytes + 1, 0xFF, 1);

  // Try to parse the Ble advertisement using our corrupted advertisement bytes.
  ByteArray corrupted_ble_advertisement_bytes{raw_ble_advertisement_bytes,
                                              kFastAdvertisementLength};
  BleAdvertisement corrupted_ble_advertisement{
      corrupted_ble_advertisement_bytes};

  EXPECT_FALSE(corrupted_ble_advertisement.IsValid());
}

}  // namespace
}  // namespace mediums
}  // namespace connections
}  // namespace nearby
}  // namespace location