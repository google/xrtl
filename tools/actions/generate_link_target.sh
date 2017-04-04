#!/bin/bash

export PYTHONPATH=$PYTHONPATH:$PWD

python tools/actions/generate_link_target.py "$@"
