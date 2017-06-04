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

#include "xrtl/testing/file_paths_map.h"

#include <fstream>

#include "xrtl/base/logging.h"

namespace xrtl {
namespace testing {

std::vector<std::pair<std::string, std::string>> FilePathsMap::file_paths_map;
std::string FilePathsMap::workspace_name;

void FilePathsMap::ParseFromManifest() {
  file_paths_map.clear();

  workspace_name = std::getenv("TEST_WORKSPACE");
  std::string manifest_path =
      std::getenv("TEST_SRCDIR") + std::string("/MANIFEST");

  // Parse relative path -> absolute path pairs line by line.
  std::ifstream manifest_file_stream(manifest_path);
  std::string relative_path;
  std::string source_absolute_path;
  while (manifest_file_stream >> relative_path >> source_absolute_path) {
    file_paths_map.push_back({relative_path, source_absolute_path});
  }
  manifest_file_stream.close();
}

StringView FilePathsMap::get_absolute_path(StringView relative_path) {
  for (auto file_path_pair : file_paths_map) {
    // Prefix relative paths with the workspace name since that's how they
    // appear in the MANIFEST file.
    if (file_path_pair.first ==
        (workspace_name + "/" + std::string(relative_path))) {
      return file_path_pair.second;
    }
  }

  return "";
}

}  // namespace testing
}  // namespace xrtl
