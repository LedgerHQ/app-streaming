#!/usr/bin/env python3

"""
https://www.cc.gatech.edu/~milos/rogers_micro07.pdf
https://www.microarch.org/micro40/talks/3B-3.pdf
https://en.wikipedia.org/wiki/Binary_search_tree
https://medium.com/ontologynetwork/everything-you-need-to-know-about-merkle-trees-82b47da0634a
https://datatracker.ietf.org/doc/html/rfc6962
"""

import hashlib


def hash(data):
    return hashlib.sha256(data).digest()


class Entry:
    """
    key: addr (4 bytes) || counter (4 bytes)
    """

    @staticmethod
    def from_values(addr, counter=0):
        return Entry(int(addr).to_bytes(4, "little") + int(counter).to_bytes(4, "little"))

    def __init__(self, data):
        assert len(data) == 8
        self.data = data
        self.addr = int.from_bytes(data[:4], "little")
        self.counter = int.from_bytes(data[4:], "little")

    def update_counter(self, counter):
        self.counter = counter
        self.data = self.data[:4] + counter.to_bytes(4, "little")

    def __bytes__(self):
        return self.data

    def __repr__(self):
        return f"<addr:{self.addr:#x}, counter:{self.counter:#x}>"


def largest_power_of_two(n):
    """https://codereview.stackexchange.com/a/105918"""
    return 1 << (n.bit_length() - 1)


def is_power_of_two(n):
    return n and (not(n & (n - 1)))


class MerkleTree:
    def __init__(self):
        self.entries = []

    def _find_index_by_addr(self, addr):
        for i, data in enumerate(self.entries):
            e = Entry(data)
            if e.addr == addr:
                return i

        return -1

    def update(self, value):
        assert type(value) == Entry

        i = self._find_index_by_addr(value.addr)
        assert i != -1

        e = Entry(self.entries[i])
        e.update_counter(value.counter)
        self.entries[i] = bytes(e)

    def insert(self, value):
        assert type(value) == Entry

        i = self._find_index_by_addr(value.addr)
        assert i == -1

        self.entries.append(bytes(value))

    def root_hash(self):
        return MerkleTree.mth(self.entries)

    @staticmethod
    def mth(entries):
        n = len(entries)
        if n == 0:
            return hash(b"")
        elif n == 1:
            return hash(b"\x00" + entries[0])
        else:
            k = largest_power_of_two(n-1)
            assert k < n and n <= 2 * k
            left = MerkleTree.mth(entries[:k])
            right = MerkleTree.mth(entries[k:])
            return hash(b"\x01" + left + right)

    @staticmethod
    def _path(m, entries):
        n = len(entries)
        if n == 1:
            return b""
        else:
            assert n > 1
            k = largest_power_of_two(n-1)
            if m < k:
                return MerkleTree._path(m, entries[:k]) + b"R" + MerkleTree.mth(entries[k:])
            else:
                return MerkleTree._path(m - k, entries[k:]) + b"L" + MerkleTree.mth(entries[:k])

    def get_proof(self, addr):
        m = self._find_index_by_addr(addr)
        assert m != -1

        proof = MerkleTree._path(m, self.entries)
        assert len(proof) % 33 == 0

        return Entry(self.entries[m]), proof

    def get_proof_of_last_entry(self):
        assert len(self.entries) > 0

        m = len(self.entries) - 1
        proof = MerkleTree._path(m, self.entries)
        assert len(proof) % 33 == 0

        return Entry(self.entries[m]), proof

    def has_addr(self, addr):
        m = self._find_index_by_addr(addr)
        return m != -1


if __name__ == "__main__":
    m = MerkleTree()

    for i in range(0, 7):
        e = Entry.from_values(i)
        m.insert(e)

    for i in range(7, 70):
        e = Entry.from_values(i)
        m.insert(e)

    e1 = Entry.from_values(0x1234, 1)
    e2 = Entry.from_values(0x1234, 2)
    m.insert(e1)
    m.update(e2)

    print(len(m.entries))
    print(m.get_proof(0x1234))
