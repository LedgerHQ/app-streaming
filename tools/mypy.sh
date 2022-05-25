#!/bin/bash

# Apply Flake8 and mypy on the whole Python code base.

set -e

find host/ app/app-*/ -type f -name '*.py' \
  ! -name '*_pb2.py' \
  -exec flake8 --max-line-length=120 '{}' '+' \
  -exec mypy --ignore-missing-imports '{}' '+'
