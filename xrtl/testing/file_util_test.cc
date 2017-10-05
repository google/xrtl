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

#include "xrtl/testing/file_util.h"

#include <unistd.h>

#include "xrtl/testing/gtest.h"

namespace xrtl {
namespace testing {
namespace {

// Tests runfiles manifest loading and resolution.
TEST(FileUtilTest, FileManifest) {
  // We rely on the test runner to load our manifest. If it fails to load then
  // the test will die before we get to this point.

  // Dump the manifest to make debugging failures easier.
  FileUtil::DumpFileManifest();

  // Resolve a path that is in the manifest and should have a runfiles path.
  absl::optional<std::string> empty_file_path_opt =
      FileUtil::ResolvePath("xrtl/testing/testdata/empty_file");
  EXPECT_TRUE(empty_file_path_opt);

  // Attempt to resolve a path that is not in the manifest and doesn't exist.
  absl::optional<std::string> invalid_file_path_opt =
      FileUtil::ResolvePath("invalid/file/path");
  EXPECT_FALSE(invalid_file_path_opt);

  // Attempt to resolve a path that is not in the manifest but does exist in the
  // workspace. We shouldn't be able to resolve it because it's not in runfiles.
  absl::optional<std::string> unsandboxed_file_path_opt =
      FileUtil::ResolvePath("xrtl/testing/testdata/BUILD");
  EXPECT_FALSE(unsandboxed_file_path_opt);
}

// Tests that output file paths are valid and writeable.
TEST(FileUtilTest, MakeOutputFilePath) {
  // Make an output file path (and ensure its directories exist).
  std::string some_output_path =
      FileUtil::MakeOutputFilePath("arbitrary/output/package/some_output");
  EXPECT_FALSE(some_output_path.empty());

  // Attempt to open the file for writing. This will fail if the directories
  // don't exist or we don't have write access to the file.
  FILE* some_output_file = std::fopen(some_output_path.c_str(), "w");
  EXPECT_NE(nullptr, some_output_file);
  std::fclose(some_output_file);
}

// Tests creating temporary files for use during tests.
TEST(FileUtilTest, MakeTempFile) {
  // Create a temp file and ensure it is cleaned up properly.
  std::string temp_file_path;
  int temp_file_fd = -1;
  {
    TempFile temp_file = FileUtil::MakeTempFile("abc");
    EXPECT_FALSE(temp_file.path().empty());
    EXPECT_NE(-1, temp_file.fd());

    // Try to write via the FD.
    FILE* file = fdopen(temp_file.fd(), "wb");
    EXPECT_NE(nullptr, file);
    std::fclose(file);

    // Try to access the file path.
    file = std::fopen(temp_file.path().c_str(), "wb");
    EXPECT_NE(nullptr, file);
    std::fclose(file);

    // Stash the values so we can test them once the TempFile leaves scope.
    temp_file_path = temp_file.path();
    temp_file_fd = temp_file.fd();
  }
  EXPECT_EQ(nullptr, std::fopen(temp_file_path.c_str(), "rb"));
  EXPECT_EQ(-1, ::dup(temp_file_fd));
}

// Tests path joining under scenarios with relative and absolute paths.
TEST(FileUtilTest, JoinPathParts) {
  EXPECT_STREQ("", FileUtil::JoinPathParts("", "").c_str());
  EXPECT_STREQ("/a", FileUtil::JoinPathParts("/a", "").c_str());
  EXPECT_STREQ("/b", FileUtil::JoinPathParts("", "/b").c_str());
  EXPECT_STREQ("a", FileUtil::JoinPathParts("a", "").c_str());
  EXPECT_STREQ("b", FileUtil::JoinPathParts("", "b").c_str());
  EXPECT_STREQ("/a/b/c", FileUtil::JoinPathParts("/a", "b/c").c_str());
  EXPECT_STREQ("/b/c", FileUtil::JoinPathParts("/a", "/b/c").c_str());
}

// Tests making full directory trees.
TEST(FileUtilTest, MakeDirectories) {
  const char* test_outdir = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR");
  std::string output_path = test_outdir ? test_outdir : "";

  // Attempt to create a file in the directory we will create below. Since it
  // shouldn't exist yet the open should fail.
  std::string output_file_path =
      FileUtil::JoinPathParts(output_path, "dir0/dir1/dir2/file");
  FILE* output_file = std::fopen(output_file_path.c_str(), "w");
  EXPECT_EQ(nullptr, output_file);

  // Make a simple directory.
  EXPECT_TRUE(FileUtil::MakeDirectories(
      FileUtil::JoinPathParts(output_path, "dir0/dir1/dir2")));

  // Redundant makes should succeed.
  EXPECT_TRUE(
      FileUtil::MakeDirectories(FileUtil::JoinPathParts(output_path, "dir0")));
  EXPECT_TRUE(FileUtil::MakeDirectories(
      FileUtil::JoinPathParts(output_path, "dir0/dir1/dir2")));

  // Now try to open a file in the output path. It should succeed.
  output_file = std::fopen(output_file_path.c_str(), "w");
  EXPECT_NE(nullptr, output_file);
  std::fclose(output_file);
}

// Tests loading testdata files.
TEST(FileUtilTest, LoadFile) {
  // Try to load a file in the manifest.
  auto empty_file_opt = FileUtil::LoadFile("xrtl/testing/testdata/empty_file");
  EXPECT_TRUE(empty_file_opt);
  EXPECT_EQ(0, empty_file_opt.value().size());

  // Load a file with actual contents.
  auto text_file_opt = FileUtil::LoadFile("xrtl/testing/testdata/text_file");
  EXPECT_TRUE(text_file_opt);
  const char kTextFileContents[] = "hello world!\n";
  EXPECT_EQ(std::strlen(kTextFileContents), text_file_opt.value().size());
  EXPECT_EQ(0, std::memcmp(text_file_opt.value().data(), kTextFileContents,
                           std::strlen(kTextFileContents)));

  // Try to load a file that is not present in the manifest.
  EXPECT_FALSE(FileUtil::LoadFile("invalid/file/path"));
}

// Tests loading text testdata files.
TEST(FileUtilTest, LoadTextFile) {
  // Try to load a file in the manifest.
  auto empty_file_opt =
      FileUtil::LoadTextFile("xrtl/testing/testdata/empty_file");
  EXPECT_TRUE(empty_file_opt);
  EXPECT_EQ(0, empty_file_opt.value().size());

  // Load a file with actual contents.
  auto text_file_opt =
      FileUtil::LoadTextFile("xrtl/testing/testdata/text_file");
  EXPECT_TRUE(text_file_opt);
  EXPECT_STREQ("hello world!\n", text_file_opt.value().c_str());

  // Try to load a file that is not present in the manifest.
  EXPECT_FALSE(FileUtil::LoadTextFile("invalid/file/path"));
}

// Tests saving files to test outputs.
TEST(FileUtilTest, SaveFile) {
  // Write with a relative path. It will be resolved automatically.
  uint8_t kOutputFileContents[3] = {0, 1, 2};
  EXPECT_TRUE(FileUtil::SaveFile("another/output/package/some_output",
                                 kOutputFileContents,
                                 sizeof(kOutputFileContents)));

  // Make the output path that was used and try to load the file from there.
  std::string some_output_path =
      FileUtil::MakeOutputFilePath("another/output/package/some_output");
  FILE* some_output_file = std::fopen(some_output_path.c_str(), "rb");
  EXPECT_NE(nullptr, some_output_file);
  uint8_t file_data[3] = {0};
  EXPECT_EQ(3, std::fread(file_data, 1, sizeof(file_data), some_output_file));
  std::fclose(some_output_file);
  EXPECT_EQ(0, std::memcmp(file_data, kOutputFileContents,
                           sizeof(kOutputFileContents)));
}

// Tests saving text test outputs.
TEST(FileUtilTest, SaveTextFile) {
  // Write with a relative path. It will be resolved automatically.
  const char kOutputFileContents[] = "hello world";
  EXPECT_TRUE(FileUtil::SaveTextFile("another/output/package/some_text_output",
                                     kOutputFileContents));

  // Make the output path that was used and try to load the file from there.
  std::string some_output_path =
      FileUtil::MakeOutputFilePath("another/output/package/some_text_output");
  FILE* some_output_file = std::fopen(some_output_path.c_str(), "rb");
  EXPECT_NE(nullptr, some_output_file);
  std::string file_data;
  file_data.resize(std::strlen(kOutputFileContents));
  EXPECT_EQ(std::strlen(kOutputFileContents),
            std::fread(&file_data[0], 1, file_data.size(), some_output_file));
  std::fclose(some_output_file);
  EXPECT_STREQ(kOutputFileContents, file_data.c_str());
}

}  // namespace
}  // namespace testing
}  // namespace xrtl
