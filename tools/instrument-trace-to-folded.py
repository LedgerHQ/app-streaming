#!/usr/bin/env python

"""
Convert a trace from an instrumented app (run with speculos) to a folded file
that can be the FlameGraph stack trace visualizer
(https://github.com/brendangregg/FlameGraph).
"""

from collections import namedtuple
from typing import Dict, Generator, List, Tuple

import os
import re
import subprocess
import sys


NO_SKIP = 0xffffffff
EXCLUDE: List[str] = [
    #"io_exchange",
]

Entry = namedtuple("Entry", "enter addr timestamp")


def get_symbols(path: str) -> Dict[int, str]:
    lines = subprocess.check_output(f"nm {path}", shell=True)
    lines = lines.decode("ascii").split("\n")
    symbols = {}
    for line in lines[:-1]:
        m = re.match("([0-9a-f]{8}) . (.*)", line)
        assert m
        addr = int(m.group(1), 16)
        addr += 0x40000000 - 0xc0de0000
        name = m.group(2)
        #print(f"{addr:#x} {name}")
        symbols[addr] = name
    return symbols


def get_excluded_addresses(symbols: Dict[int, str]):
    excluded_addresses = []

    for addr, name in symbols.items():
        if name in EXCLUDE:
            excluded_addresses.append(addr)

    return excluded_addresses


def read_trace(path: str, symbols: Dict[int, str]) -> Generator[Entry, Entry, None]:
    with open(path, "r") as fp:
        lines = fp.readlines()

    excluded_addresses = get_excluded_addresses(symbols)

    level = 0
    skip_until = NO_SKIP

    for line in lines:
        m = re.match("([<>]) ([0-9a-f]{8})\.([0-9a-f]{8}) ([0-9a-f]{8}) ([0-9a-f]{8})", line)
        if not m:
            continue
        tv_sec = int(m.group(2), 16)
        tv_usec = int(m.group(3), 16)
        assert tv_usec < 1000000
        timestamp = tv_sec * 1000000 + tv_usec
        addr = int(m.group(4), 16)
        enter = m.group(1)

        if enter == "<":
            level -= 1

            if skip_until == level:
                skip_until = NO_SKIP
        else:
            if addr in excluded_addresses and skip_until == NO_SKIP:
                skip_until = level

        if level <= skip_until:
            yield Entry(enter, addr, timestamp)

        if enter == ">":
            level += 1


def print_trace(trace: Generator[Entry, Entry, None]):
    """Pretty-print a trace to stdout."""

    level = 0
    for entry in trace:
        if entry.enter == "<":
            level -= 1
            #symbol = symbols.get(entry.addr, entry.addr)
            #print(f"{' ' * level * 2} {symbol}")
        else:
            symbol = symbols.get(entry.addr, entry.addr)
            print(f"{' ' * level * 2} {symbol}")

            level += 1


def count_occurences(trace) -> None:
    """Sort and display the number of occurences of each symbol to stdout."""

    stats: Dict[int, int] = {}
    for entry in trace:
        if entry.enter != ">":
            continue
        if entry.addr not in stats:
            stats[entry.addr] = 1
        else:
            stats[entry.addr] += 1

    sorted_stats: List[Tuple[int, int]] = [(addr, n) for (addr, n) in stats.items()]
    sorted_stats.sort(key=lambda tup: tup[1], reverse=True)

    for addr, n in sorted_stats:
        symbol = symbols.get(addr, addr)
        print(f"{n} {symbol}")


def print_folded(trace: Generator[Entry, Entry, None]) -> None:
    """Print a trace into the folded format to stdout."""

    stacktrace: List[str] = []
    timestamp: List[int] = []
    durations: List[int] = []
    level = 0

    for entry in trace:
        if entry.enter == "<":
            end = entry.timestamp
            start = timestamp.pop()
            duration = end - start
            assert end >= start

            child_duration = durations.pop()
            assert child_duration <= duration
            durations[-1] += duration
            duration -= child_duration

            # try to ignore the time consumed by the instrumentation
            duration = max(0, duration - 2)

            print(f"{';'.join(stacktrace)} {duration}")

            symbol = stacktrace.pop()
            #print(f"{entry.enter} {' ' * level * 2} {symbol}")

            level -= 1
        else:
            level += 1
            symbol = symbols.get(entry.addr, f"{entry.addr:#x}")
            #print(f"{entry.enter} {' ' * level * 2} {symbol}")
            stacktrace.append(symbol)
            timestamp.append(entry.timestamp)
            durations.append(0)
            #print(f"+ {';'.join(stacktrace)}")


if __name__ == "__main__":
    symbols = get_symbols("vm/bin/app.elf")
    trace = read_trace(sys.argv[1], symbols)

    #print_trace(trace)
    #count_occurences(trace)
    print_folded(trace)
