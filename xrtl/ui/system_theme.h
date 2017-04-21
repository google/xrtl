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

#ifndef XRTL_UI_SYSTEM_THEME_H_
#define XRTL_UI_SYSTEM_THEME_H_

#include "xrtl/base/macros.h"

namespace xrtl {
namespace ui {

// Provides access to system metrics.
// Note that the user may change the system metrics at any time. Listen for the
// Window or Control OnSystemThemeChanged events and update cached values.
//
// Platform references:
// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724385(v=vs.85).aspx
//  MacOS:
//  iOS: create a UIView, get default values
//  X11: TBD
//  Win32: GetSystemMetrics
class SystemMetrics {
 public:
};

// Provides access to system colors.
// Note that the user may change the system colors at any time. Listen for the
// Window or Control OnSystemThemeChanged events and update cached values.
//
// Platform references:
//  MacOS: GetThemeBrushAsColor
//  iOS: create a UIView, get default values
//  X11: TBD
//  Win32: GetSysColor
class SystemColors {
 public:
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_SYSTEM_THEME_H_
