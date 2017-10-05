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

#ifndef XRTL_TESTING_FILE_UTIL_H_
#define XRTL_TESTING_FILE_UTIL_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "absl/types/optional.h"

namespace xrtl {
namespace testing {

// Temporary file handle.
// The file is opened for binary output and available via fd. It also exists
// at the path and can be reopened from that for as long as the TempFile
// instance is alive.
class TempFile {
 public:
  TempFile(std::string path, int fd);
  ~TempFile();

  // Absolute path to the temp file.
  // The file is deleted when the TempFile instance is destroyed.
  const std::string& path() const { return path_; }

  // File handle opened in write/binary.
  int fd() const { return fd_; }

 private:
  std::string path_;
  int fd_ = 0;
};

// Mapping from test data relative paths to absolute paths.
// If runfiles are fully supported, relative paths should work on their own.
class FileUtil {
 public:
  // Parses the file paths map from the runfiles MANIFEST file.
  // Call this during test setup.
  //
  // On Linux and MacOS the MANIFEST file is not present in the sandbox and will
  // not be used. Instead, all paths passed to ResolvePath will be returned
  // verbatim iff the file exists in the runfiles directory (likely as a
  // symlink). On Windows where the MANIFEST is present it will be loaded and
  // used for resolution.
  // See https://github.com/bazelbuild/bazel/issues/3726 for why the MANIFEST
  // file is not safe to sandbox.
  static void LoadFileManifest(const std::string& executable_path);

  // Dumps the file manifest mappings to stdout for debugging.
  static void DumpFileManifest();

  // Gets the path for the provided test data relative path.
  // Only files specified in the "data" field of tests will be available here
  // and any other request will return false.
  static absl::optional<std::string> ResolvePath(
      absl::string_view relative_path);

  // Returns a new absolute file path that can be used for test outputs.
  static std::string MakeOutputFilePath(absl::string_view base_name);

  // Returns a new absolute file path to a temp file that will not appear in
  // test outputs.
  static TempFile MakeTempFile(absl::string_view base_name);

  // Joins two path parts following standard path rules.
  // Usage:
  //   JoinPathParts("/a", "b/c") -> "/a/b/c"
  //   JoinPathParts("/a", "/b/c") -> "/b/c"
  static std::string JoinPathParts(absl::string_view part_a,
                                   absl::string_view part_b);

  // Ensures the directory at the given absolute path exists, creating it as
  // needed.
  // Returns true if the directory exists (either prior to the call or after).
  static bool MakeDirectories(absl::string_view path);

  // Loads a file from the given path.
  // The path will be resolved using the loaded file manifest, if any.
  static absl::optional<std::vector<uint8_t>> LoadFile(absl::string_view path);

  // Loads a text file from the given path.
  // The path will be resolved using the loaded file manifest, if any.
  // The returned string may have trailing whitespace.
  static absl::optional<std::string> LoadTextFile(absl::string_view path);

  // Saves a file to the given path.
  // This output path may be a relative path in which case MakeOutputFilePath is
  // used to choose its final location. If an absolute path is provided (such as
  // from TempFile::path) it will be used verbatim.
  static bool SaveFile(absl::string_view path,
                       const std::vector<uint8_t>& data);
  static bool SaveFile(absl::string_view path, const void* data,
                       size_t data_length);

  // Saves a text file to the given path.
  static bool SaveTextFile(absl::string_view path,
                           absl::string_view text_value);

 private:
  struct FileManifest {
    // True if the MANIFEST file is present and the path mappings are valid.
    bool is_present = false;
    std::vector<std::pair<std::string, std::string>> path_mappings;
  };
  static FileManifest* file_manifest_;
};

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_FILE_UTIL_H_
