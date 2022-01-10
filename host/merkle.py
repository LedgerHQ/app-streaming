#!/usr/bin/env python3

"""
This is an experiment with Merkle Trees. Not used by the project.

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
    key: addr (3 bytes) || counter (4 bytes)
    """

    def __init__(self, data):
        assert len(data) == 7
        self.data = data
        self.addr = int.from_bytes(data[:3], "little")
        self.counter = int.from_bytes(data[3:], "little")

    def update_counter(self, counter):
        self.counter = counter
        self.data = self.data[:3] + counter.to_bytes(4, "little")

    def __bytes__(self):
        return self.data


def largest_power_of_two(n):
    """https://codereview.stackexchange.com/a/105918"""
    return 1 << (n.bit_length() - 1)


def is_power_of_two(n):
    return n and (not(n & (n - 1)))


def bit_count(self):
    return bin(self).count("1")


class MerkleTree:
    def __init__(self):
        self.entries = []

    @staticmethod
    def mth(entries):
        n = len(entries)
        if n == 0:
            return hash(b"")
        elif n == 1:
            return hash(b"\x00" + bytes(entries[0]))
        else:
            k = largest_power_of_two(n-1)
            assert k < n and n <= 2 * k
            left = MerkleTree.mth(entries[:k])
            right = MerkleTree.mth(entries[k:])
            return hash(b"\x01" + left + right)

    def compute_next_root_hash(self, prev_root_hash, value):
        if len(self.entries) == 0:
            next_root_hash = hash(b"\x00" + bytes(value))
            return next_root_hash

        last_value = self.entries[-1]
        proof = self.get_proof(last_value)

        # verify the proof

        assert prev_root_hash == MerkleTree._proof_hash(proof, last_value)

        # a subtree will be created
        tree_level = len(proof) // 33 - (bit_count(len(self.entries)) - 1)
        proof_before = proof[:33*tree_level]
        proof_after = proof[33*tree_level:]

        digest = MerkleTree._proof_hash(proof_before, bytes(last_value))
        tmp_digest = MerkleTree._proof_hash(b"L" + digest, bytes(value))
        next_root_hash = MerkleTree._proof_hash(proof_after, None, previous=tmp_digest)

        return next_root_hash

    def insert(self, value):
        if value in self.entries:
            return

        prev_root_hash = self.root_hash()

        if type(value) == Entry:
            for e in self.entries:
                if type(e) == Entry and e.addr == value.addr:
                    proof = self.get_proof(e)

                    # verify the proof
                    assert prev_root_hash == MerkleTree._proof_hash(proof, bytes(e))

                    next_root_hash = MerkleTree._proof_hash(proof, bytes(value))

                    # update value
                    e.update_counter(value.counter)

                    assert self.root_hash() == next_root_hash

                    return

        next_root_hash = self.compute_next_root_hash(prev_root_hash, value)

        self.entries.append(value)

        # verify next_root_hash
        assert self.root_hash() == next_root_hash

    def root_hash(self):
        return MerkleTree.mth(self.entries)

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

    def get_proof(self, value):
        assert value in self.entries

        m = self.entries.index(value)
        proof = MerkleTree._path(m, self.entries)
        assert len(proof) % 33 == 0

        return proof

    @staticmethod
    def _proof_hash(proof, value, previous=None):
        if previous is not None:
            assert value is None
        else:
            previous = hash(b"\x00" + value)

        assert len(proof) % 33 == 0

        for i in range(0, len(proof), 33):
            c = chr(proof[i])
            digest = proof[i+1:i+1+32]
            if c == "R":
                left = previous
                right = digest
            elif c == "L":
                left = digest
                right = previous
            else:
                assert False
            assert len(left) == 32
            assert len(right) == 32
            previous = hash(b"\x01" + left + right)

        return previous

    def verify(self, value):
        proof = self.get_proof(value)
        previous = MerkleTree._proof_hash(proof, value)
        assert previous == self.root_hash()


if __name__ == "__main__":
    m = MerkleTree()

    for i in range(0, 7):
        m.insert(f"d{i}".encode())

    #print(f"[*] root hash: {m.root_hash().hex()}")
    #print(m.get_proof(b"d6"))

    for value in m.entries:
        m.verify(value)

    for i in range(7, 70):
        m.insert(f"d{i}".encode())

    m.verify(b"d68")
    #print(m.get_proof(b"d68"))

    e1 = Entry(int(0x1234).to_bytes(3, "little") + int(1).to_bytes(4, "little"))
    e2 = Entry(int(0x1234).to_bytes(3, "little") + int(2).to_bytes(4, "little"))
    m.insert(e1)
    m.insert(e2)
    print(len(m.entries))
