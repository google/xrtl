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

#include "xrtl/base/cli_main.h"

#include <string>

#include "xrtl/base/flags.h"
#include "xrtl/base/logging.h"

namespace xrtl {

int CliMain(int argc, char** argv) {
  // Setup flags.
  xrtl::flags::SetUsageMessage(std::string("\n$ ") + argv[0]);
  xrtl::flags::ParseCommandLineFlags(&argc, &argv, true);

  int exit_code = GetCliEntryPoint()(argc, argv);

  xrtl::flags::ShutDownCommandLineFlags();

  return exit_code;
}

}  // namespace xrtl
