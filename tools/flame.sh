#!/bin/bash

set -e

./tools/instrument-trace-to-folded.py /tmp/trace.txt > /tmp/trace.folded
./tools/FlameGraph/flamegraph.pl /tmp/trace.folded > /tmp/trace.svg
sed -i 's/width="1200" height="[[:digit:]]\+"/width="100%" height=""/' /tmp/trace.svg
echo ok
