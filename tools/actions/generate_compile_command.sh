#!/bin/bash

export PYTHONPATH=$PYTHONPATH:$PWD

python tools/actions/generate_compile_command.py "$@"
