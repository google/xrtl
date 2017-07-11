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

#ifndef XRTL_UI_INPUT_EVENTS_H_
#define XRTL_UI_INPUT_EVENTS_H_

#include <cstdint>

#include "xrtl/base/geometry.h"
#include "xrtl/base/macros.h"
#include "xrtl/ui/key_codes.h"

namespace xrtl {
namespace ui {

// A bitmask of keyboard modifier keys that may be pressed at any given time.
enum class ModifierKey {
  kNone = 0,
  kShift = 1 << 0,
  kCtrl = 1 << 1,
  kAlt = 1 << 2,
  kSuper = 1 << 3,
};
XRTL_BITMASK(ModifierKey);

// Base UI event type.
class InputEvent {
 public:
  // A bitmask of modifiers keys pressed when the event was emitted.
  ModifierKey modifier_key_mask() const { return modifier_key_mask_; }
  // Returns true if the shift key was held during the event.
  bool is_shift_pressed() const {
    return any(modifier_key_mask_ & ModifierKey::kShift);
  }
  // Returns true if the ctrl key was held during the event.
  bool is_ctrl_pressed() const {
    return any(modifier_key_mask_ & ModifierKey::kCtrl);
  }
  // Returns true if the alt key was held during the event.
  bool is_alt_pressed() const {
    return any(modifier_key_mask_ & ModifierKey::kAlt);
  }

 protected:
  explicit InputEvent(ModifierKey modifier_key_mask)
      : modifier_key_mask_(modifier_key_mask) {}

  ModifierKey modifier_key_mask_ = ModifierKey::kNone;
};

// A keyboard key event.
class KeyboardEvent : public InputEvent {
 public:
  KeyboardEvent(int key_code, VirtualKey virtual_key,
                ModifierKey modifier_key_mask)
      : InputEvent(modifier_key_mask),
        key_code_(key_code),
        virtual_key_(virtual_key) {}

  // TODO(benvanik): normalize into some logical keymap. Value is undefined.
  int key_code() const { return key_code_; }

  // Virtual key code normalized across platforms.
  VirtualKey virtual_key() const { return virtual_key_; }

 private:
  int key_code_ = 0;
  VirtualKey virtual_key_ = VirtualKey::kNone;
};

// Mouse button enumeration.
enum class MouseButton {
  // No button was pressed.
  kNone = 0,
  // Left button (on right-handed mouses).
  kButton1 = 1 << 0,
  kLeftButton = kButton1,
  // Middle button.
  kButton2 = 1 << 1,
  kMiddleButton = kButton2,
  // Right button (on right-handed mouses).
  kButton3 = 1 << 2,
  kRightButton = kButton3,
  // Alt button 1 (browser back).
  kButton4 = 1 << 3,
  // Alt button 2 (browser forward).
  kButton5 = 1 << 4,
};
XRTL_BITMASK(MouseButton);

// A mouse cursor event.
class MouseEvent : public InputEvent {
 public:
  MouseEvent(Point2D screen_offset_px, Point2D control_offset_px,
             int wheel_delta, MouseButton action_button,
             MouseButton pressed_button_mask, ModifierKey modifier_key_mask)
      : InputEvent(modifier_key_mask),
        screen_offset_px_(screen_offset_px),
        control_offset_px_(control_offset_px),
        wheel_delta_(wheel_delta),
        action_button_(action_button),
        pressed_button_mask_(pressed_button_mask) {}

  // Mouse cursor offset on the screen in pixels.
  // The coordinates may be negative on multi-monitor systems.
  const Point2D& screen_offset_px() const { return screen_offset_px_; }
  // Mouse cursor offset in the target control in pixels.
  const Point2D& control_offset_px() const { return control_offset_px_; }

  // Mouse wheel delta.
  // TODO(benvanik): document range.
  int wheel_delta() const { return wheel_delta_; }

  // Which button triggered this event.
  MouseButton action_button() const { return action_button_; }
  // A bitmask of all buttons currently pressed.
  MouseButton pressed_button_mask() const { return pressed_button_mask_; }

 private:
  Point2D screen_offset_px_;
  Point2D control_offset_px_;
  int wheel_delta_;
  MouseButton action_button_ = MouseButton::kNone;
  MouseButton pressed_button_mask_ = MouseButton::kNone;
};

}  // namespace ui
}  // namespace xrtl

#endif  // XRTL_UI_INPUT_EVENTS_H_
