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

"""Generates a link_targets.json database.

Usage:
  bazel --experimental_action_listener=//tools/actions:generate_link_targets_listener
"""

import argparse
import fnmatch
import os.path
import subprocess
import sys


def _get_link_target_from_file(file_path, command_directory):
  """Gets a link target for a single executable.

  Args:
    file_path: The path to a *_compile_command file.
    command_directory: The directory commands are run from.
  Returns:
    A string to append to link_targets.json or None if the file was empty.
  """
  with open(file_path, 'r') as f:
    contents = f.read().split('\0')
    if len(contents) < 3:
      return None
    target_package = contents[0]
    target_uuid = contents[1]
    target_executable = os.path.join(command_directory, contents[2])
    return '''{
      "package": "%s",
      "uuid": "%s",
      "executable": "%s"
    },''' % (
        target_package,
        target_uuid,
        target_executable.replace('\\', '\\\\'),
    )


def _get_link_targets(path, command_directory):
  """Recursively iterates all paths and gets the commands for all files within.

  Args:
    path: A directory path to look for *_link_target files under.
    command_directory: The directory commands are run from.

  Returns:
    A list of strings to append to compile_commands.json.
  """
  all_targets = []
  for root, dirnames, filenames in os.walk(path):
    for filename in fnmatch.filter(filenames, '*_link_target'):
      file_path = os.path.join(root, filename)
      target = _get_link_target_from_file(file_path, command_directory)
      if target:
        all_targets.append(target)
  return all_targets


def main():
  parser = argparse.ArgumentParser(prog='generate_link_targets_json')
  parser.add_argument('--workspace_root', help='.')
  parser.add_argument('--execution_root', help='bazel info execution_root')
  parser.add_argument('--build_root', help='bazel-out/[config]/')
  parser.add_argument('--output_file', default='link_targets.json',
                      help='Output file path for the database file.')

  # If the user passed no args, die nicely.
  if len(sys.argv) == 1:
    parser.print_help()
    return 1
  args = vars(parser.parse_args(sys.argv[1:]))

  action_outs = os.path.join(args['build_root'],
                             'extra_actions',
                             'tools/actions/generate_link_targets_action')
  action_outs = action_outs.replace('/', os.sep)
  targets = _get_link_targets(action_outs, args['execution_root'])
  with open(args['output_file'], 'w') as f:
    f.write('[')
    for target in targets:
      f.write(target)
    # Delete the last comma to make the file valid JSON.
    f.seek(f.tell()-1)
    f.write(']')
  return 0


if __name__ == '__main__':
  sys.exit(main())
