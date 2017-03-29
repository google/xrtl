#include <emscripten.h>

#include <cstdio>
#include <cstdlib>

#include "xrtl/base/logging.h"
#include "xrtl/port/common/base/logging_macros.h"

namespace xrtl {

void FlushLog() {}

namespace internal {

#define EM_LOG_FLAGS (EM_LOG_DEMANGLE | EM_LOG_CONSOLE)
#define EM_LOG_FLAGS_STACK \
  (EM_LOG_FLAGS | EM_LOG_JS_STACK | EM_LOG_C_STACK | EM_LOG_NO_PATHS)

void LogMessage::EmitLogMessage() {
  int emscripten_log_level;
  switch (severity()) {
    case INFO:
      emscripten_log_level = EM_LOG_FLAGS;
      break;
    case WARNING:
      emscripten_log_level = EM_LOG_WARN | EM_LOG_FLAGS;
      break;
    case ERROR:
      emscripten_log_level = EM_LOG_ERROR | EM_LOG_FLAGS;
      break;
    case FATAL:
      emscripten_log_level = EM_LOG_ERROR | EM_LOG_FLAGS_STACK;
      break;
    default:
      if (severity_ < INFO) {
        emscripten_log_level = EM_LOG_FLAGS;
      } else {
        emscripten_log_level = EM_LOG_ERROR | EM_LOG_FLAGS;
      }
      break;
  }

  std::stringstream ss;
  const char* const partial_name = strrchr(file_name(), '/');
  ss << (partial_name != nullptr ? partial_name + 1 : file_name()) << ":"
     << line() << " " << str();
  emscripten_log(emscripten_log_level, ss.str().c_str());

  // Emscripten logging at level FATAL does not terminate execution, so abort()
  // is still required to stop the program.
  if (severity() == FATAL) {
    abort();
  }
}

}  // namespace internal
}  // namespace xrtl
