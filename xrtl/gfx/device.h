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

#ifndef XRTL_GFX_DEVICE_H_
#define XRTL_GFX_DEVICE_H_

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

#include "xrtl/base/ref_ptr.h"

namespace xrtl {
namespace gfx {

// The type of the device.
enum class DeviceType : uint32_t {
  // A CPU (either the primary CPU or some CPU-like accelerator).
  kCpu = 1 << 0,
  // A GPU of some kind.
  kGpu = 1 << 1,
  // A virtualized GPU (such as in a virtualization environment).
  kGpuVirtual = kGpu | (1 << 2),
  // A GPU embedded or tightly coupled with the primary CPU.
  kGpuIntegrated = kGpu | (1 << 3),
  // A GPU separate from from the CPU.
  kGpuDiscrete = kGpu | (1 << 4),
  // Something else or unknown.
  kOther = 1 << 5,
};
XRTL_BITMASK(DeviceType);

// A device available for use by the backend graphics API.
// This may represent a physical device in the system or a logical device as
// exposed by the API.
//
// For more information on device limits on each API/platform, see:
//   OpenGL ES 3.0:
//     http://opengles.gpuinfo.org/gles_devicefeatures.php
//     https://www.g-truc.net/doc/OpenGL%20ES%203%20Hardware%20Matrix.pdf
//   Vulkan:
//     http://vulkan.gpuinfo.org/listlimits.php
//   Metal:
//     https://developer.apple.com/metal/Metal-Feature-Set-Tables.pdf
//   D3D12:
//     https://msdn.microsoft.com/en-us/library/windows/desktop/mt186615(v=vs.85).aspx
class Device : public RefObject<Device> {
 public:
  virtual ~Device() = default;

  // Describes the limits of the device.
  //
  // Maps to:
  //  https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPhysicalDeviceLimits.html
  struct Limits {
    // TODO(benvanik): texture sizes, buffer sizes, render-target counts, etc.

    // Maximum number of ResourceSets that are available for binding.
    // Shaders with set indices larger than this value will fail to bind.
    // | ES3 4 | VK 4 | MTL ∞ | D3D ∞ |
    int resource_set_count = 4;

    // Maximum duration of a QueueFence timeout in nanoseconds. Any timeout
    // provided will be clamped to this value.
    std::chrono::nanoseconds max_queue_fence_timeout_nanos;
  };

  // Describes the features available for use on the device.
  // When passed to CreateContext it is used to enable specific features on the
  // created context.
  //
  // Key:
  //   .: not supported
  //   ~: optional, but practically not supported
  //   ?: optional, often supported
  //   ✔: always supported
  //
  // Maps to:
  //  https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkPhysicalDeviceFeatures.html
  struct Features {
    // TODO(benvanik): robust buffer access, full draw index uint32, extensions.
    // TODO(benvanik): render or compute.

    // Defines which pixel formats are available for use on the device.
    // Any format not covered by the flags below can be assumed always present.
    // Note that not all formats support use as a render target.
    struct PixelFormats {
      // Supports the packed kD24UNormS8UInt format.
      // | ES3 ✔ | VK ✔ | MTL ~ | D3D ✔ |
      bool packed_depth_stencil = false;
      // Supports the BC1, BC2, and BC3 formats.
      // | ES3 ~ | VK ✔ | MTL ~ | D3D ✔ |
      bool bc1_2_3 = false;
      // Supports the BC4, BC5, BC6, and BC7 formats.
      // | ES3 . | VK ✔ | MTL ~ | D3D ✔ |
      bool bc4_5_6_7 = false;
      // Supports the ETC2 compressed texture formats.
      // | ES3 ✔ | VK ~ | MTL . | D3D . |
      bool etc2 = false;
      // Supports the EAC compressed texture formats.
      // | ES3 ✔ | VK ~ | MTL ? | D3D . |
      bool eac = false;
      // Supports the ASTC compressed texture formats.
      // | ES3 ~ | VK ~ | MTL ? | D3D . |
      bool astc = false;
      // Supports the PVRTC(1) compressed texture formats.
      // | ES3 ~ | VK . | MTL ✔ | D3D . |
      bool pvrtc = false;
    } pixel_formats;
  };

  // Describes a queue family available on the device.
  // Each queue family supports one or more capabilities and may have one or
  // more independent queues that can operate in parallel. Each queue within a
  // family can be retrieved from the Context after creation as a Queue object.
  //
  // Maps to:
  //  https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkQueueFamilyProperties.html
  struct QueueFamily {
    // Internal queue family identifier.
    int queue_family_index = 0;
    // True if the queues support render operations.
    bool supports_render = false;
    // True if the queues support compute operations.
    bool supports_compute = false;
    // True if the queues support transfer operations.
    bool supports_transfer = false;
    // Total number of queues that may be created from this family.
    int queue_count = 0;
    // True if the queue supports timing queries.
    bool has_timing_support = false;
  };

  // The type of the device.
  DeviceType type() const { return type_; }
  // Whether the device is a GPU.
  bool is_gpu() const { return (type_ & DeviceType::kGpu) == DeviceType::kGpu; }
  // A vendor-unique identifier (such as '123') or empty string.
  const std::string& vendor_id() const { return vendor_id_; }
  // A vendor name (such as 'NVIDIA') or empty string.
  const std::string& vendor_name() const { return vendor_name_; }
  // A vendor-specific identifier (such as '123') or empty string.
  const std::string& device_id() const { return device_id_; }
  // A vendor-specific device name (such as 'GeForce Blah') or empty string.
  const std::string& device_name() const { return device_name_; }
  // A driver version string (such as '1.2.3') or empty string.
  const std::string& driver_version() const { return driver_version_; }

  // An identifier unique within the ContextFactory that can be used to identify
  // devices that are compatible with each other and can be used to create a
  // multi-device context.
  // For example if device A has ID 1 and device B has ID 2 they are
  // incompatible and cannot be used together. If both devices shared an ID of 1
  // they could be used together but a device C with ID 2 could not be.
  int multi_device_group_id() const { return multi_device_group_id_; }

  // Limits of the device. Attempting to use values out of these ranges will
  // result in failures that are difficult to detect so always check first.
  const Limits& limits() const { return limits_; }

  // Available device features for use by the context.
  const Features& features() const { return features_; }

  // A list of the queue families and capabilities available on the device.
  const std::vector<QueueFamily>& queue_families() const {
    return queue_families_;
  }

  // Returns true if the set of required features is satisfiable from the set
  // of available features on the device.
  bool IsCompatible(const Features& requested_features) const;

 protected:
  Device() = default;

  DeviceType type_ = DeviceType::kOther;
  std::string vendor_id_;
  std::string vendor_name_;
  std::string device_id_;
  std::string device_name_;
  std::string driver_version_;
  int multi_device_group_id_ = 0;
  Limits limits_;
  Features features_;
  std::vector<QueueFamily> queue_families_;
};

}  // namespace gfx
}  // namespace xrtl

#endif  // XRTL_GFX_DEVICE_H_
