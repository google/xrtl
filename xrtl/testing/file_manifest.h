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

#ifndef XRTL_TESTING_FILE_MANIFEST_H_
#define XRTL_TESTING_FILE_MANIFEST_H_

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"

namespace xrtl {
namespace testing {

// Mapping from test data relative paths to absolute paths.
// If runfiles are fully supported, relative paths should work on their own.
class FileManifest {
 public:
  // Parses the file paths map from the runfiles MANIFEST file.
  // Call this during test setup.
  static void ParseFromManifest(const std::string& executable_path);

  // Gets the path for the provided test data relative path.
  // Returns the input relative path without modifications if the exact relative
  // path is not found in the runfiles MANIFEST.
  // Only files specified in the "data" field of tests will be available here.
  static absl::string_view ResolvePath(absl::string_view relative_path);

 private:
  static std::vector<std::pair<std::string, std::string>> file_paths_map;
  static std::string workspace_name;
};

}  // namespace testing
}  // namespace xrtl

#endif  // XRTL_TESTING_FILE_MANIFEST_H_
