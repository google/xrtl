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

#ifndef XRTL_GFX_MANAGED_OBJECT_H_
#define XRTL_GFX_MANAGED_OBJECT_H_

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace gfx {

// Base type for objects managed by the graphics system.
// These objects may have differing lifetime rules based on the underlying
// implementation.
class ManagedObject : public RefObject<ManagedObject> {
 public:
  virtual ~ManagedObject() = default;

  // Deleter used by RefObject instead of normal C++ delete.
  static void Delete(ManagedObject* obj) { obj->Release(); }

 protected:
  ManagedObject() = default;

  // Implementation subclasses may override this to perform custom destruction
  // logic, such as returning to a pool or deferring deletion to another thread.
  virtual void Release() { delete this; }
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_MANAGED_OBJECT_H_
