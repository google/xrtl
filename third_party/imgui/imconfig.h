#ifndef IMCONFIG_H_
#define IMCONFIG_H_

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#include "xrtl/base/logging.h"

// Define assertion handler.
#define IM_ASSERT(_EXPR) DCHECK(_EXPR)

// Don't define obsolete functions names.
#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

// Define constructor and implicit cast operators to convert glm.
#define IM_VEC2_CLASS_EXTRA    \
  ImVec2(const glm::vec2& f) { \
    x = f.x;                   \
    y = f.y;                   \
  }                            \
  operator glm::vec2() const { return glm::vec2(x, y); }
#define IM_VEC4_CLASS_EXTRA    \
  ImVec4(const glm::vec4& f) { \
    x = f.x;                   \
    y = f.y;                   \
    z = f.z;                   \
    w = f.w;                   \
  }                            \
  operator glm::vec4() const { return glm::vec4(x, y, z, w); }

struct ImGuiContext;
extern thread_local ImGuiContext* thread_imgui_context;

#define GImGui thread_imgui_context
#define IMGUI_SET_CURRENT_CONTEXT_FUNC(ctx) thread_imgui_context = ctx

#endif  // IMCONFIG_H_
