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

#ifndef XRTL_BASE_CLI_MAIN_H_
#define XRTL_BASE_CLI_MAIN_H_

namespace xrtl {

using CliEntryPointFn = int (*)(int argc, char** argv);

// Returns a function pointer to the entry point of the application.
// CliMain will call this once the environment has been setup.
//
// Usage:
//  my_cli.cc:
//   int MyEntry(int argc, char** argv) { return 55; }
//   DECLARE_CLI_ENTRY_POINT(MyEntry);
CliEntryPointFn GetCliEntryPoint();

#define DECLARE_CLI_ENTRY_POINT(fn) \
  xrtl::CliEntryPointFn xrtl::GetCliEntryPoint() { return fn; }

// Application entry point for CLI apps.
// This follows the standard C main() function convention.
int CliMain(int argc, char** argv);

}  // namespace xrtl

#endif  // XRTL_BASE_CLI_MAIN_H_
