#!/bin/bash

# Apply Flake8 and mypy on the whole Python code base.

set -e

find host/ -type f -name '*.py' \
  -exec flake8 --max-line-length=120 '{}' '+' \
  -exec mypy --ignore-missing-imports '{}' '+'
