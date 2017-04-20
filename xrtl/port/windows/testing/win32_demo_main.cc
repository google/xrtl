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

#include <string>
#include <vector>

#include "xrtl/base/logging.h"
#include "xrtl/port/windows/base/windows.h"
#include "xrtl/testing/demo_main.h"

namespace xrtl {
namespace testing {

// Entry point when using /SUBSYSTEM:CONSOLE.
extern "C" int main(int argc, char** argv) { return DemoMain(argc, argv); }

// Entry point when using /SUBSYSTEM:WINDOWS.
extern "C" int WINAPI WinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
  // Convert command line to an argv-like format.
  // NOTE: the command line that comes in with the WinMain arg is garbage.
  int argc = 0;
  wchar_t** argv_w = ::CommandLineToArgvW(::GetCommandLineW(), &argc);
  if (!argc || !argv_w) {
    LOG(FATAL) << "Unable to parse command line";
    return 1;
  }

  // Convert all args to narrow char strings.
  std::vector<std::string> allocated_strings(argc);
  std::vector<char*> argv_a(argc);
  for (int i = 0; i < argc; ++i) {
    size_t char_length = wcslen(argv_w[i]);
    allocated_strings[i].resize(char_length);
    argv_a[i] = const_cast<char*>(allocated_strings[i].data());
    std::wcstombs(argv_a[i], argv_w[i], char_length + 1);
  }
  ::LocalFree(argv_w);

  // Setup COM on the main thread.
  // NOTE: this may fail if COM has already been initialized - that's OK.
  ::CoInitializeEx(nullptr, COINIT_MULTITHREADED);

  // Run common demo main.
  int exit_code = DemoMain(argc, argv_a.data());

  // Release arg memory.
  argv_a.clear();
  allocated_strings.clear();

  return exit_code;
}

}  // namespace testing
}  // namespace xrtl

// Add manifest dependencies directly in code.
// The manifests allow us to opt into newer UI features and are needed by most
// anything we try to do with the system (file pickers, etc).
// This is done instead of trying to embed xml resources, which is nasty.
// https://msdn.microsoft.com/en-us/library/windows/desktop/bb773175(v=vs.85).aspx
#pragma comment( \
    linker,      \
    "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")  // NOLINT(whitespace/line_length)
