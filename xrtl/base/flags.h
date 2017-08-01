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

#ifndef XRTL_BASE_FLAGS_H_
#define XRTL_BASE_FLAGS_H_

#include <string>
#include <vector>

#include "xrtl/base/macros.h"

#ifdef XRTL_CONFIG_GOOGLE_INTERNAL
#include "xrtl/port/google/base/flags_forward.h"
#else
#include "xrtl/port/common/base/gflags_flags.h"
#endif  // XRTL_CONFIG_GOOGLE_INTERNAL

namespace xrtl {
namespace flags {

// Set the "usage" message for this program.
// For example:
//   std::string usage("This program does nothing. Sample usage:\n");
//   usage += argv[0] + " <uselessarg1> <uselessarg2>";
//   SetUsageMessage(usage);
// Do not include commandline flags in the usage: we do that for you!
// Thread-hostile; meant to be called before any threads are spawned.
void SetUsageMessage(const std::string& usage);

// Sets the version string, which is emitted with --version.
// For instance: SetVersionString("1.3");
// Thread-hostile; meant to be called before any threads are spawned.
void SetVersionString(const std::string& version);

// Looks for flags in argv and parses them.
// Rearranges argv to put flags first, or removes them entirely if remove_flags
// is true. If a flag is defined more than once in the command line or flag
// file the last definition is used.
// Returns the index (into argv) of the first non-flag argument.
uint32_t ParseCommandLineFlags(int* argc, char*** argv, bool remove_flags);

// Shuts down the command line flags system and reclaims all memory.
// This should be called before exiting to make leak checkers happy.
void ShutDownCommandLineFlags();

// Gets a list of all flags by name.
std::vector<std::string> GetAllFlagNames();

// Gets the current value of a flag by name.
// Returns true if the flag was found and out_value is valid.
bool GetFlagValue(const char* flag_name, std::string* out_value);

// Gets the current value of a flag by name.
// Returns the flag value or the given default if the flag is not found.
std::string GetFlagValue(const char* flag_name,
                         const std::string& default_value);

// Sets a flag by name with the given string value.
// Returns true if the flag was found and set.
bool SetFlagValue(const char* flag_name, const char* flag_value);

}  // namespace flags
}  // namespace xrtl

#endif  // XRTL_BASE_FLAGS_H_
