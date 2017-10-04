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

#include "xrtl/testing/file_manifest.h"

#include <algorithm>
#include <fstream>

namespace xrtl {
namespace testing {

std::vector<std::pair<std::string, std::string>> FileManifest::file_paths_map;

void FileManifest::ParseFromManifest(const std::string& executable_path) {
  file_paths_map.clear();

  // TEST_SRCDIR will point to runfiles when running under bazel test.
  const char* test_srcdir = std::getenv("TEST_SRCDIR");
  std::string runfiles_path = test_srcdir ? test_srcdir : "";
  if (runfiles_path.empty()) {
    // Running outside of bazel test. Use module path to infer runfiles.
    size_t last_slash =
        std::min(executable_path.rfind('/'), executable_path.rfind('\\'));
    if (last_slash != std::string::npos) {
      std::string executable_parent = executable_path.substr(0, last_slash);
      std::string executable_name = executable_path.substr(last_slash + 1);
      runfiles_path = executable_parent + "/" + executable_name + ".runfiles";
    }
  }
  std::string manifest_path = runfiles_path + std::string("/MANIFEST");

  // Parse relative path -> absolute path pairs line by line.
  std::ifstream manifest_file_stream(manifest_path);
  std::string relative_path;
  std::string source_absolute_path;
  while (manifest_file_stream >> relative_path >> source_absolute_path) {
    file_paths_map.push_back({relative_path, source_absolute_path});
  }
  manifest_file_stream.close();
}

absl::string_view FileManifest::ResolvePath(absl::string_view relative_path) {
  std::string target_path = std::string(std::getenv("TEST_WORKSPACE")) + "/" +
                            std::string(relative_path);
  for (auto file_path_pair : file_paths_map) {
    // Prefix relative paths with the workspace name since that's how they
    // appear in the MANIFEST file.
    if (file_path_pair.first == target_path) {
      return file_path_pair.second;
    }
  }

  return relative_path;
}

}  // namespace testing
}  // namespace xrtl
