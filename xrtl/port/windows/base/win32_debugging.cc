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

#include <crtdbg.h>
#include <fcntl.h>
#include <io.h>

#include "xrtl/base/debugging.h"
#include "xrtl/base/logging.h"
#include "xrtl/port/windows/base/windows.h"

namespace xrtl {
namespace debugging {

namespace {

// Redirects the given named standard output handle to the attached console.
// Returns the redirected file handle or nullptr if an error occurred.
FILE* RedirectOutputHandle(DWORD std_handle, const char* name) {
  HANDLE handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
  if (handle == INVALID_HANDLE_VALUE) {
    LOG(ERROR) << "Failed to get " << name << " handle";
    return nullptr;
  } else if (!handle) {
    LOG(ERROR) << "No attached console";
    return nullptr;
  }
  // Associate handle with a C file descriptor.
  int fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_TEXT);
  if (fd == -1) {
    LOG(ERROR) << "Failed to associate " << name << " handle to fd";
    return nullptr;
  }
  // Open the new file descriptor for writing.
  FILE* file = _fdopen(fd, "w");
  if (!file) {
    LOG(ERROR) << "Failed to open output for " << name;
    return nullptr;
  }
  // Disable buffering so there's no delay.
  setvbuf(file, nullptr, _IONBF, 0);
  return file;
}

}  // namespace

void EnableDebugHeap() {
  int flag_value = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
  // Enable debug heap allocations:
  flag_value |= _CRTDBG_ALLOC_MEM_DF;
  // Check heap integrity on every memory operation:
  flag_value |= _CRTDBG_CHECK_ALWAYS_DF;
  // Check CRT memory (not just app memory):
  // flag_value |= _CRTDBG_CHECK_CRT_DF;
  // Delay frees for a bit:
  // flag_value |= _CRTDBG_DELAY_FREE_MEM_DF;
  // Print leak checking on exit:
  flag_value |= _CRTDBG_LEAK_CHECK_DF;
  _CrtSetDbgFlag(flag_value);
}

bool is_console_attached() {
  return ::GetFileType(::GetStdHandle(STD_OUTPUT_HANDLE)) != 0;
}

bool AttachConsole() {
  if (is_console_attached()) {
    // Already attached (to something).
    return true;
  }

  bool has_console = ::AttachConsole(ATTACH_PARENT_PROCESS) == TRUE;
  if (!has_console) {
    // We weren't launched from a console so allocate a new one and send our
    // output there.
    if (::AllocConsole() == TRUE) {
      FILE* file;
      freopen_s(&file, "CONOUT$", "w", stdout);
      freopen_s(&file, "CONOUT$", "w", stderr);
      return true;
    } else {
      LOG(WARNING) << "Console could not be allocated";
      return false;
    }
  }

  // Get the handle for STDOUT.
  FILE* stdout_handle = RedirectOutputHandle(STD_OUTPUT_HANDLE, "STDOUT");
  FILE* stderr_handle = RedirectOutputHandle(STD_ERROR_HANDLE, "STDERR");
  if (!stdout_handle || !stderr_handle) {
    LOG(WARNING) << "Console redirection disabled";
    return false;
  }
  *stdout = *stdout_handle;
  *stderr = *stderr_handle;

  return true;
}

bool is_debugger_attached() { return ::IsDebuggerPresent() ? true : false; }

}  // namespace debugging
}  // namespace xrtl
