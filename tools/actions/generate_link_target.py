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

"""A Bazel extra_action which genenerates link target files.
The files written are consumed by the generate_link_targets_json.py script
to collapse them into a single database.
"""

import sys

import third_party.bazel.extra_actions_base_pb2 as extra_actions_base_pb2


def main(argv):
  action = extra_actions_base_pb2.ExtraActionInfo()
  with open(argv[1], 'rb') as f:
    action.MergeFromString(f.read())

  # From CppLink:
  cpp_link_info = action.Extensions[extra_actions_base_pb2.CppLinkInfo.cpp_link_info]
  if cpp_link_info.link_target_type != 'EXECUTABLE':
    # Skip all STATIC_LIBRARYs.
    with open(argv[2], 'w') as f:
      pass
    return 0

  # Write out required data for generate_link_targets_json.py.
  with open(argv[2], 'w') as f:
    # //some/path:rule
    f.write(action.owner)
    f.write('\0')
    # 1517a....
    f.write(action.id)
    f.write('\0')
    # bazel-out/.../some.exe
    f.write(cpp_link_info.output_file)


if __name__ == '__main__':
  sys.exit(main(sys.argv))
