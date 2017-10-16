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

#include "xrtl/testing/file_util.h"

#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "xrtl/base/logging.h"
#include "xrtl/base/tracing.h"
#include "xrtl/tools/target_platform/target_platform.h"

#if defined(XRTL_PLATFORM_WINDOWS)
#include <direct.h>
#define PLATFORM_MKDIR(p, unused) ::_mkdir(p)
#define PLATFORM_STAT _stat
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#else
#include <unistd.h>
#define PLATFORM_MKDIR ::mkdir
#define PLATFORM_STAT stat
#endif  // XRTL_PLATFORM_WINDOWS

namespace xrtl {
namespace testing {

namespace {

// Returns true if the given path is /absolute.
bool IsPathAbsolute(absl::string_view path) {
  if (path.size() > 0 && path[0] == '/') {
    // Unix /absolute/path.
    return true;
  } else if (path.size() > 2 && path[1] == ':') {
    // Windows C:\absolute\path.
    return true;
  }
  return false;
}

}  // namespace

TempFile::TempFile(std::string path, int fd)
    : path_(std::move(path)), fd_(fd) {}

TempFile::~TempFile() {
  if (fd_ != -1) {
    ::close(fd_);
  }
  ::unlink(path_.c_str());
}

FileUtil::FileManifest* FileUtil::file_manifest_ = nullptr;

void FileUtil::LoadFileManifest(const std::string& executable_path) {
  WTF_SCOPE0("FileUtil#LoadFileManifest");

  if (file_manifest_) {
    delete file_manifest_;
    file_manifest_ = nullptr;
  }

  // TODO(benvanik): atexit deleter; ASAN seems ok with this.
  file_manifest_ = new FileManifest();

  // TEST_SRCDIR will point to runfiles when running under bazel test.
  const char* test_srcdir = std::getenv("TEST_SRCDIR");
  std::string runfiles_path = test_srcdir ? test_srcdir : "";
  if (runfiles_path.empty()) {
    // Running outside of bazel test. Use module path to infer runfiles.
    size_t last_slash =
        std::min(executable_path.rfind('/'), executable_path.rfind('\\'));
    if (last_slash != std::string::npos) {
      std::string executable_parent = executable_path.substr(0, last_slash);
      std::string executable_name = executable_path.substr(last_slash + 1);
      runfiles_path = executable_parent + "/" + executable_name + ".runfiles";
    }
  }
  std::string manifest_path = JoinPathParts(runfiles_path, "MANIFEST");

  // Parse relative path -> absolute path pairs line by line.
  // NOTE: it's ok if the MANIFEST file is not found and this will just no-op.
  std::ifstream manifest_file_stream(manifest_path);
  if (manifest_file_stream.is_open()) {
    file_manifest_->is_present = true;
    std::string relative_path;
    std::string source_absolute_path;
    while (manifest_file_stream >> relative_path >> source_absolute_path) {
      file_manifest_->path_mappings.push_back(
          {relative_path, source_absolute_path});
    }
    manifest_file_stream.close();
  }
}

void FileUtil::DumpFileManifest() {
  CHECK(file_manifest_) << "File manifest not loaded";
  if (file_manifest_->is_present) {
    for (auto file_path_pair : file_manifest_->path_mappings) {
      LOG(INFO) << file_path_pair.first << " -> " << file_path_pair.second;
    }
  } else {
    // TODO(benvanik): if the manifest was not found walk the runfiles path and
    //                 dump all files present.
  }
}

absl::optional<std::string> FileUtil::ResolvePath(
    absl::string_view relative_path) {
  CHECK(file_manifest_) << "File manifest not loaded";
  CHECK(!IsPathAbsolute(relative_path)) << "Cannot resolve absolute paths";

  // Scan the MANIFEST if present and otherwise check the filesystem.
  if (file_manifest_->is_present) {
    // Prefix relative paths with the workspace name since that's how they
    // appear in the MANIFEST file.
    std::string target_path =
        JoinPathParts(std::getenv("TEST_WORKSPACE"), relative_path);
    for (auto file_path_pair : file_manifest_->path_mappings) {
      if (file_path_pair.first == target_path) {
        return file_path_pair.second;
      }
    }
  } else {
    // NOTE: this does touch a file for each path resolved, but we aren't
    // particularly sensitive to this overhead in tests.
    std::string runfiles_relative_path = std::string(relative_path);
    struct PLATFORM_STAT stat_buffer;
    if (PLATFORM_STAT(runfiles_relative_path.c_str(), &stat_buffer) == 0) {
      return runfiles_relative_path;
    }
  }

  // Not found - prevent access to the unsandboxed file path.
  return absl::nullopt;
}

std::string FileUtil::MakeOutputFilePath(absl::string_view base_name) {
  // TEST_UNDECLARED_OUTPUTS_DIR will point to a writeable path when running
  // under bazel where outputs should be placed. These will get saved during
  // test runs on CIs so they can be viewed later.
  const char* test_outdir = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR");
  std::string output_path = test_outdir ? test_outdir : "";

  // TODO(benvanik): fallback to C++ temp path.
  CHECK(!output_path.empty()) << "TEST_UNDECLARED_OUTPUTS_DIR not specified";

  std::string output_file_path = JoinPathParts(output_path, base_name);

  // Ensure output path exists.
  std::string output_base_path =
      output_file_path.substr(0, output_file_path.rfind('/'));
  bool output_path_exists = MakeDirectories(output_base_path);
  CHECK(output_path_exists) << "Unable to make output base path at "
                            << output_base_path;

  return output_file_path;
}

TempFile FileUtil::MakeTempFile(absl::string_view base_name) {
  // TEST_TMPDIR will point to a writeable temp path when running under bazel.
  const char* test_tmpdir = std::getenv("TEST_TMPDIR");
  std::string tmp_path = test_tmpdir ? test_tmpdir : "";

  // TODO(benvanik): fallback to C++ temp path.
  CHECK(!tmp_path.empty()) << "TEST_TMPDIR not specified";

  std::string template_path = JoinPathParts(tmp_path, base_name) + "XXXXXX";

  int fd = -1;
#if defined(XRTL_PLATFORM_WINDOWS)
  if (::_mktemp(&template_path[0]) != nullptr) {
    fd = ::open(template_path.c_str(), O_CREAT | O_EXCL | O_RDWR | O_BINARY,
                S_IRUSR | S_IWUSR);
  }
#else
  fd = ::mkstemp(&template_path[0]);
#endif  // XRTL_PLATFORM_WINDOWS
  if (fd == -1) {
    LOG(FATAL) << "Failed to create temporary file";
  }

  return TempFile(template_path, fd);
}

std::string FileUtil::JoinPathParts(absl::string_view part_a,
                                    absl::string_view part_b) {
  if (part_a.empty()) {
    return std::string(part_b);
  } else if (part_b.empty()) {
    return std::string(part_a);
  } else if (part_b.front() == '/') {
    return std::string(part_b);
  } else {
    if (part_a.back() == '/') {
      std::string result = std::string(part_a);
      absl::StrAppend(&result, part_b);
      return result;
    } else {
      return absl::StrJoin({part_a, part_b}, "/");
    }
  }
}

bool FileUtil::MakeDirectories(absl::string_view path) {
  // Walk each path part and ensure it exists.
  std::vector<absl::string_view> path_parts = absl::StrSplit(path, '/');
  std::string current_path;
  for (size_t i = 0; i < path_parts.size(); ++i) {
    if (path_parts[i].empty()) continue;
    absl::StrAppend(&current_path, "/", path_parts[i]);
    int mkdir_result = PLATFORM_MKDIR(current_path.c_str(), S_IRWXU);
    if (mkdir_result == -1 && errno != EEXIST) {
      LOG(ERROR) << "Failed to create test directory at " << current_path << " "
                 << mkdir_result << " (for full path " << path << ")";
      return false;
    }
  }
  return true;
}

absl::optional<std::vector<uint8_t>> FileUtil::LoadFile(
    absl::string_view path) {
  WTF_SCOPE0("FileUtil#LoadFile");

  // Open the file at its properly resolved path.
  absl::optional<std::string> resolved_path_opt =
      IsPathAbsolute(path) ? std::string(path) : ResolvePath(path);
  if (!resolved_path_opt) {
    LOG(ERROR) << "File not found: " << path;
    return absl::nullopt;
  }
  std::string resolved_path = resolved_path_opt.value();
  FILE* file = std::fopen(resolved_path.c_str(), "rb");
  if (!file) {
    LOG(ERROR) << "Unable to open file " << path
               << " (resolved: " << resolved_path << ")";
    return absl::nullopt;
  }

  // Determine length.
  std::fseek(file, 0, SEEK_END);
  size_t total_size = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);

  // Read into buffer.
  std::vector<uint8_t> data_buffer;
  data_buffer.resize(total_size);
  std::fread(data_buffer.data(), data_buffer.size(), 1, file);
  std::fclose(file);
  return data_buffer;
}

absl::optional<std::string> FileUtil::LoadTextFile(absl::string_view path) {
  auto file_contents_opt = LoadFile(path);
  if (!file_contents_opt) {
    return absl::nullopt;
  }
  const auto& file_contents = file_contents_opt.value();

  std::string string_contents;
  string_contents.resize(file_contents.size());
  std::memcpy(&string_contents[0], file_contents.data(), file_contents.size());
  return string_contents;
}

bool FileUtil::SaveFile(absl::string_view path,
                        const std::vector<uint8_t>& data) {
  return SaveFile(path, data.data(), data.size());
}

bool FileUtil::SaveFile(absl::string_view path, const void* data,
                        size_t data_length) {
  WTF_SCOPE0("FileUtil#SaveFile");

  // Open file.
  std::string resolved_path =
      IsPathAbsolute(path) ? std::string(path) : MakeOutputFilePath(path);
  FILE* file = std::fopen(resolved_path.c_str(), "wb");
  if (!file) {
    LOG(ERROR) << "Unable to open file " << path << " for writing";
    return false;
  }

  // Write entire data buffer into the file.
  std::fwrite(data, data_length, 1, file);
  std::fclose(file);

  return true;
}

bool FileUtil::SaveTextFile(absl::string_view path,
                            absl::string_view text_value) {
  return SaveFile(path, text_value.data(), text_value.size());
}

}  // namespace testing
}  // namespace xrtl
