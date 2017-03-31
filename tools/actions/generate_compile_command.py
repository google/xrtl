#!/usr/bin/env python

# Copyright 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""A Bazel extra_action which genenerates LLVM compilation database files.
The files written are consumed by the generate_compile_commands_json.py script
to collapse them into a single database.
"""

import sys

import third_party.bazel.extra_actions_base_pb2 as extra_actions_base_pb2


# When set commands will be emitted for all .h files required per unit.
INCLUDE_ALL_HEADERS = True


def _get_cpp_command(cpp_compile_info):
  compiler = cpp_compile_info.tool
  options = ' '.join(cpp_compile_info.compiler_option)
  source = cpp_compile_info.source_file
  if len(cpp_compile_info.sources_and_headers) and INCLUDE_ALL_HEADERS:
    sources = [source] + \
              [source_file for source_file in cpp_compile_info.sources_and_headers
               if source_file.endswith('.h')]
  else:
    sources = [source]
  output = cpp_compile_info.output_file
  return '%s %s -c %s -o %s' % (compiler, options, source, output), sources


def main(argv):
  action = extra_actions_base_pb2.ExtraActionInfo()
  with open(argv[1], 'rb') as f:
    action.MergeFromString(f.read())
  command, source_files = _get_cpp_command(
      action.Extensions[extra_actions_base_pb2.CppCompileInfo.cpp_compile_info])
  with open(argv[2], 'w') as f:
    for source_file in source_files:
      f.write(command)
      f.write('\0')
      f.write(source_file)
      f.write('\0')


if __name__ == '__main__':
  sys.exit(main(sys.argv))
