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

#ifndef XRTL_UI_KEY_CODES_H_
#define XRTL_UI_KEY_CODES_H_

#include <cstdint>

namespace xrtl {
namespace ui {

// Maximum value of a VirtualKey value.
constexpr int kVirtualKeyCount = 256;

// Virtual key codes.
// These are normalized across all platforms.
//
// The values are taken from the Windows VK_* constants, as they are generally
// the most sane and easiest to find mappings for:
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
enum class VirtualKey : uint8_t {
  kNone = 0x00,

  kBackspace = 0x08,  // aka back
  kTab = 0x09,

  // 0x0A - 0x0B: reserved.

  kClear = 0x0C,
  kEnter = 0x0D,  // aka return

  // 0x0E - 0x0F: undefined.

  kShift = 0x10,
  kControl = 0x11,
  kAlt = 0x12,  // aka menu
  kPause = 0x13,
  kCapsLock = 0x14,  // aka capital

  kImeKana = 0x15,
  kImeHangul = kImeKana,
  kImeJunja = 0x17,
  kImeFinal = 0x18,
  kImeKanji = 0x19,
  kImeHanja = kImeKanji,

  kEscape = 0x1B,

  kImeConvert = 0x1C,
  kImeNonConvert = 0x1D,
  kImeAccept = 0x1E,
  kImeModeChange = 0x1F,

  kSpace = 0x20,
  kPageUp = 0x21,    // aka prior
  kPageDown = 0x22,  // aka next
  kEnd = 0x23,
  kHome = 0x24,
  kLeft = 0x25,
  kUp = 0x26,
  kRight = 0x27,
  kDown = 0x28,
  kSelect = 0x29,
  kPrint = 0x2A,
  kExecute = 0x2B,
  kPrintScreen = 0x2C,  // aka snapshot
  kInsert = 0x2D,
  kDelete = 0x2E,
  kHelp = 0x2F,

  k0 = 0x30,
  k1 = 0x31,
  k2 = 0x32,
  k3 = 0x33,
  k4 = 0x34,
  k5 = 0x35,
  k6 = 0x36,
  k7 = 0x37,
  k8 = 0x38,
  k9 = 0x39,

  // 0x40: unassigned.

  kA = 0x41,
  kB = 0x42,
  kC = 0x43,
  kD = 0x44,
  kE = 0x45,
  kF = 0x46,
  kG = 0x47,
  kH = 0x48,
  kI = 0x49,
  kJ = 0x4A,
  kK = 0x4B,
  kL = 0x4C,
  kM = 0x4D,
  kN = 0x4E,
  kO = 0x4F,
  kP = 0x50,
  kQ = 0x51,
  kR = 0x52,
  kS = 0x53,
  kT = 0x54,
  kU = 0x55,
  kV = 0x56,
  kW = 0x57,
  kX = 0x58,
  kY = 0x59,
  kZ = 0x5A,

  kLeftMeta = 0x5B,   // aka lwin
  kRightMeta = 0x5C,  // aka rwin
  kApps = 0x5D,

  // 0x5E: reserved.

  kSleep = 0x5F,

  kNumpad0 = 0x60,
  kNumpad1 = 0x61,
  kNumpad2 = 0x62,
  kNumpad3 = 0x63,
  kNumpad4 = 0x64,
  kNumpad5 = 0x65,
  kNumpad6 = 0x66,
  kNumpad7 = 0x67,
  kNumpad8 = 0x68,
  kNumpad9 = 0x69,
  kNumpadMultiply = 0x6A,  // aka multiply
  kNumpadAdd = 0x6B,       // aka add
  kNumpadSlash = 0x6C,     // aka separator
  kNumpadSubtract = 0x6D,  // aka subtract
  kNumpadDecimal = 0x6E,   // aka decimal
  kNumpadDivide = 0x6F,    // aka divide

  kF1 = 0x70,
  kF2 = 0x71,
  kF3 = 0x72,
  kF4 = 0x73,
  kF5 = 0x74,
  kF6 = 0x75,
  kF7 = 0x76,
  kF8 = 0x77,
  kF9 = 0x78,
  kF10 = 0x79,
  kF11 = 0x7A,
  kF12 = 0x7B,
  kF13 = 0x7C,
  kF14 = 0x7D,
  kF15 = 0x7E,
  kF16 = 0x7F,
  kF17 = 0x80,
  kF18 = 0x81,
  kF19 = 0x82,
  kF20 = 0x83,
  kF21 = 0x84,
  kF22 = 0x85,
  kF23 = 0x86,
  kF24 = 0x87,

  // 0x88 - 0x8F: unassigned.

  kNumLock = 0x90,
  kScrollLock = 0x91,  // aka scroll

  // 0x92 - 0x97: unimportant weird keyboards.
  // 0x97 - 0x9F: unassigned.

  kLeftShift = 0xA0,
  kRightShift = 0xA1,
  kLeftControl = 0xA2,
  kRightControl = 0xA3,
  kLeftMenu = 0xA4,
  kRightMenu = 0xA5,

  kBrowserBack = 0xA6,
  kBrowserForward = 0xA7,
  kBrowserRefresh = 0xA8,
  kBrowserStop = 0xA9,
  kBrowserSearch = 0xAA,
  kBrowserFavorites = 0xAB,
  kBrowserHome = 0xAC,

  kVolumeMute = 0xAD,
  kVolumeDown = 0xAE,
  kVolumeUp = 0xAF,
  kMediaNextTrack = 0xB0,
  kMediaPrevTrack = 0xB1,
  kMediaStop = 0xB2,
  kMediaPlayPause = 0xB3,
  kLaunchMail = 0xB4,
  kLaunchMediaSelect = 0xB5,
  kLaunchApp1 = 0xB6,
  kLaunchApp2 = 0xB7,

  // 0xB8 - 0xB9: reserved.

  // TODO(benvanik): ignore these?
  kOem1 = 0xBA,       // ';:' for US
  kOemPlus = 0xBB,    // '+' any country
  kOemComma = 0xBC,   // ',' any country
  kOemMinus = 0xBD,   // '-' any country
  kOemPeriod = 0xBE,  // '.' any country
  kOem2 = 0xBF,       // '/?' for US
  kOem3 = 0xC0,       // '`~' for US

  // 0xC1 - 0xD7: reserved.
  // 0xD8 - 0xDA: unassigned.

  // TODO(benvanik): ignore these?
  kOem4 = 0xDB,  //  '[{' for US
  kOem5 = 0xDC,  //  '\|' for US
  kOem6 = 0xDD,  //  ']}' for US
  kOem7 = 0xDE,  //  ''"' for US
  kOem8 = 0xDF,  //

  // 0xE0: reserved.
  // 0xE1 - 0xE6: extended/enhanced keys.
  // 0xE7: packet.
  // 0xE8: unassigned.
  // 0xE9 - 0xF5: OEM specific.
  // 0xF6 - 0xFE: Weird keys.
  // 0xFF: reserved.
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_KEY_CODES_H_
