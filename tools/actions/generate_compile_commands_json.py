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

"""Generates a compile_commands.json database.

Usage:
  bazel --experimental_action_listener=//tools/actions:generate_compile_commands_listener
"""

import argparse
import fnmatch
import os.path
import subprocess
import sys


def _get_commands_from_file(file_path, command_directory):
  """Gets a set of commands for a single file.

  Args:
    file_path: The path to a *_compile_command file.
    command_directory: The directory commands are run from.
  Returns:
    A string to append to compile_commands.json.
  """
  with open(file_path, 'r') as f:
    contents = f.read().split('\0')
    commands = []
    for i in range(0, len(contents) / 2):
      commands.append('''{
        "directory": "%s",
        "command": "%s",
        "file": "%s"
      },''' % (
          command_directory,
          contents[i * 2].replace('"', '\\"'),
          os.path.abspath(contents[i * 2 + 1]),
      ))
    return commands


def _get_compile_commands(path, command_directory):
  """Recursively iterates all paths and gets the commands for all files within.

  Args:
    path: A directory path to look for *_compile_command files under.
    command_directory: The directory commands are run from.
  
  Returns:
    A list of strings to append to compile_commands.json.
  """
  all_commands = []
  for root, dirnames, filenames in os.walk(path):
    for filename in fnmatch.filter(filenames, '*_compile_command'):
        file_path = os.path.join(root, filename)
        commands = _get_commands_from_file(file_path, command_directory)
        if commands:
          all_commands.extend(commands)
  # for file_path in path.iterdir():
  #   if os.path.isdir(file_path):
  #     all_commands.extend(_get_compile_commands(file_path, command_directory))
  #   elif f.name.endswith('_compile_command'):
  #     commands = _get_commands_from_file(file_path, command_directory)
  #     if commands:
  #       all_commands.extend(commands)
  return all_commands


def main():
  parser = argparse.ArgumentParser(prog='generate_compile_commands_json')
  parser.add_argument('--output_file', default='compile_commands.json',
                      help='Output file path for the database file.')

  # If the user passed no args, die nicely.
  if len(sys.argv) == 1:
    parser.print_help()
    return 1
  args = vars(parser.parse_args(sys.argv[1:]))

  source_path = os.path.join(os.path.dirname(__file__), '../..')
  action_outs = os.path.join(source_path,
                             'bazel-bin/../extra_actions',
                             'tools/actions/generate_compile_commands_action')
  command_directory = subprocess.check_output(
      ('bazel', 'info', 'execution_root'),
      cwd=source_path).decode('utf-8').rstrip()
  commands = _get_compile_commands(action_outs, command_directory)
  with open(args['output_file'], 'w') as f:
    f.write('[')
    for command in commands:
      f.write(command)
    # Delete the last comma to make the file valid JSON.
    f.seek(f.tell()-1)
    f.write(']')
  return 0


if __name__ == '__main__':
  sys.exit(main())
