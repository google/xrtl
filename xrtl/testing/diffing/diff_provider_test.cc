// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "xrtl/testing/diffing/diff_provider.h"

#include "xrtl/testing/file_util.h"
#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {
namespace diffing {
namespace {

const char kGoldenBasePath[] = "xrtl/testing/diffing/testdata/goldens";

class DiffProviderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    diff_provider_ = DiffProvider::Create();
    EXPECT_TRUE(diff_provider_);
    EXPECT_TRUE(diff_provider_->Initialize(kGoldenBasePath));
  }

  void TearDown() override { diff_provider_.reset(); }

  std::unique_ptr<DiffProvider> diff_provider_;
};

// Tests that a diff provider can be created and initialized.
TEST_F(DiffProviderTest, Initialization) { EXPECT_TRUE(diff_provider_); }

// Tests comparing text.
TEST_F(DiffProviderTest, CompareText) {
  // Try a known match.
  auto test_text_buffer =
      FileUtil::LoadTextFile("xrtl/testing/diffing/testdata/text_file.txt")
          .value();
  EXPECT_EQ(DiffResult::kEquivalent,
            diff_provider_->CompareText("text_file", test_text_buffer,
                                        DiffPublishMode::kNever, {}));

  // Try a known mismatch.
  auto test_text_mismatch_buffer =
      FileUtil::LoadTextFile(
          "xrtl/testing/diffing/testdata/text_file_mismatch.txt")
          .value();
  EXPECT_EQ(DiffResult::kDifferent,
            diff_provider_->CompareText("text_file", test_text_mismatch_buffer,
                                        DiffPublishMode::kNever, {}));

  // Try a missing reference.
  EXPECT_EQ(DiffResult::kMissingReference,
            diff_provider_->CompareText("text_file_missing", test_text_buffer,
                                        DiffPublishMode::kNever, {}));
}

// Tests comparing data.
TEST_F(DiffProviderTest, CompareData) {
  // Try a known match.
  auto test_data_buffer =
      FileUtil::LoadFile("xrtl/testing/diffing/testdata/data_file.bin").value();
  EXPECT_EQ(DiffResult::kEquivalent,
            diff_provider_->CompareData("data_file", test_data_buffer,
                                        DiffPublishMode::kNever, {}));

  // Try a known mismatch.
  auto test_data_mismatch_buffer =
      FileUtil::LoadFile("xrtl/testing/diffing/testdata/data_file_mismatch.bin")
          .value();
  EXPECT_EQ(DiffResult::kDifferent,
            diff_provider_->CompareData("data_file", test_data_mismatch_buffer,
                                        DiffPublishMode::kNever, {}));

  // Try a missing reference.
  EXPECT_EQ(
      DiffResult::kMissingReference,
      diff_provider_->CompareData("test_data_file_missing", test_data_buffer,
                                  DiffPublishMode::kNever, {}));
}

// Tests comparing images.
TEST_F(DiffProviderTest, CompareImage) {
  // Try a known match.
  auto test_image_buffer =
      ImageBuffer::Load("xrtl/testing/diffing/testdata/image_file.png", 3);
  EXPECT_EQ(DiffResult::kEquivalent,
            diff_provider_->CompareImage("image_file", test_image_buffer.get(),
                                         DiffPublishMode::kNever, {}));

  // Try a known mismatch.
  auto test_image_mismatch_buffer = ImageBuffer::Load(
      "xrtl/testing/diffing/testdata/image_file_mismatch.png", 3);
  EXPECT_EQ(DiffResult::kDifferent,
            diff_provider_->CompareImage("image_file",
                                         test_image_mismatch_buffer.get(),
                                         DiffPublishMode::kNever, {}));

  // Try a missing reference.
  EXPECT_EQ(DiffResult::kMissingReference,
            diff_provider_->CompareImage("image_file_missing",
                                         test_image_buffer.get(),
                                         DiffPublishMode::kNever, {}));
}

}  // namespace
}  // namespace diffing
}  // namespace testing
}  // namespace xrtl
