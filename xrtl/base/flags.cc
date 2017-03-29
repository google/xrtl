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

#include "xrtl/base/flags.h"

namespace xrtl {
namespace flags {

void SetUsageMessage(const std::string& usage) {
  gflags::SetUsageMessage(usage);
}

void SetVersionString(const std::string& version) {
  gflags::SetVersionString(version);
}

uint32_t ParseCommandLineFlags(int* argc, char*** argv, bool remove_flags) {
  return gflags::ParseCommandLineFlags(argc, argv, remove_flags);
}

std::vector<std::string> GetAllFlagNames() {
  std::vector<gflags::CommandLineFlagInfo> all_flags;
  gflags::GetAllFlags(&all_flags);
  std::vector<std::string> flag_names;
  flag_names.reserve(all_flags.size());
  for (const auto& flag_info : all_flags) {
    flag_names.push_back(flag_info.name);
  }
  return flag_names;
}

bool GetFlagValue(const char* flag_name, std::string* out_value) {
  *out_value = "";
  return gflags::GetCommandLineOption(flag_name, out_value);
}

std::string GetFlagValue(const char* flag_name,
                         const std::string& default_value) {
  std::string value;
  if (gflags::GetCommandLineOption(flag_name, &value)) {
    return value;
  } else {
    return default_value;
  }
}

bool SetFlagValue(const char* flag_name, const char* flag_value) {
  // TODO(benvanik): detect flag presence - may need to use Get.
  gflags::SetCommandLineOption(flag_name, flag_value);
  return true;
}

}  // namespace flags
}  // namespace xrtl
