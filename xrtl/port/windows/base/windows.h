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

#ifndef XRTL_PORT_WINDOWS_BASE_WINDOWS_H_
#define XRTL_PORT_WINDOWS_BASE_WINDOWS_H_

// This file brings in the common Windows headers.
// It should only be included by files under port/windows/.

#include "xrtl/tools/target_platform/target_platform.h"

#ifndef XRTL_PLATFORM_WINDOWS
#error Attempting to use Windows headers on a non-Windows platform.
#endif  // !XRTL_PLATFORM_WINDOWS

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX

// Bazel's default CROSSTOOL defines NOGDI. Undo that nastiness.
#undef NOGDI

// Must be first.
#include <windows.h>

#include <mmsystem.h>  // winmm
#include <objbase.h>   // ole32
#include <shellapi.h>  // shell32

// Global namespace conflicts.
#undef ERROR

#endif  // XRTL_PORT_WINDOWS_BASE_WINDOWS_H_
